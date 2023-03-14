<?php

namespace App\Http\Controllers;

use Illuminate\Auth\Access\AuthorizationException;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Cache;
use Illuminate\Support\Facades\Http;
use Illuminate\Support\Facades\Validator;

class SocialStats extends Controller
{
    //starting from post body data
    public function post(Request $request) {
        try {
            $socials = $request->all();
            $validator = Validator::make($socials, [
                "*.t" => "required|in:y,t,i",
                "*.c" => "required|min:4|max:60"
            ]);
            if($validator->fails())
                return response()->json(["message" => "wrong request format. Instead use [{'t': 'y' | 't', 'c': %channelname%}]"], 400);

            $socials = $this->retrieveSocials($socials);

            return response()->json($socials);
        } catch(\Exception $e) {
            return response()->json(["message" => "could not process requested channel data"], 500);
        }
    }

    /**
     * It takes an array of socials, retrieves the stats from the cache if they exist, else it requests
     * them from the API, then it returns the socials with the stats
     *
     * @param array socials An array of socials to retrieve.
     *
     * @return An array of socials with the following keys:
     * - t: The type of social (y for youtube, t for twitch, i for instagram)
     * - c: The channel name
     * - d: The display name
     * - f: The number of followers/subs
     * - v: The number of views/follows
     */
    private function retrieveSocials(array $socials) {
        $socials = collect($socials);

        //Try getting youtube socials from cache else request from youtube
        $youtube = $socials->filter(function($social) {
            return $social["t"] == "y";
        })->map(function($social) {
            try {
                $social["id"] = $this->getYoutubeChannelId($social["c"]);
            } catch(\Exception) {
                $social["id"] = "";
            }
            $social["v"] = Cache::get("youtube_channel_views_".$social["id"]);
            $social["f"] = Cache::get("youtube_channel_subs_".$social["id"]);

            return $social;
        });

        $youtubeResponse = null;
        if(!$youtube->every(function($social) {
            return $social["v"] != null && $social["f"] != null;
        })) {
            try {
                $youtubeResponse = collect($this->requestYoutubeChannelStats($youtube->map(function($social){ return $social["id"]; })->toArray()));
            } catch(\Exception) {}
        }

        //Try getting twitch socials from cache else request from twitch
        $twitch = $socials->filter(function($social) {
            return $social["t"] == "t";
        })->map(function($social) {
            try {
                $social["id"] = $this->getTwitchUserId($social["c"]);
            } catch(\Exception) {
                $social["id"] = "";
            }
            $social["v"] = Cache::get("twitch_user_views_".$social["id"]);
            $social["f"] = Cache::get("twitch_user_follower_".$social["id"]) ?? $this->getTwitchFollower($social["id"]);

            return $social;
        });

        $twitchResponse = null;
        if(!$twitch->every(function($social) {
            return $social["v"] != null;
        })) {
            try {
                $twitchResponse = collect($this->requestTwitchViewers($twitch->map(function($social){ return $social["id"]; })->toArray()));
            } catch(\Exception) {}
        }

        //Try getting instagram socials from cache or else request from instagram
        $socials = $socials->map(function($social) {
            if($social["t"] == "i") {
                $social["f"] = Cache::get("instagram_user_follower_".$social["c"]);
                $social["v"] = Cache::get("instagram_user_following_".$social["c"]);

                if(is_null($social["f"]) && is_null($social["v"])) {
                    try {
                        $instagramData = $this->requestInstagramStats($social["c"]);
                        $social["id"] = $instagramData["id"];
                        $social["f"] = $instagramData["f"];
                        $social["v"] = $instagramData["v"];
                    } catch(\Exception) {}
                }
            }

            return $social;
        });

        //Merge socials
        return $socials->map(function($social) use ($youtube, $twitch, $youtubeResponse, $twitchResponse) {
            switch($social["t"]) {
                case "y":
                    $social = array_merge($social, $youtube->firstOrFail(function($channel) use ($social) { return $channel["c"] == $social["c"]; }));
                    if(isset($youtubeResponse))
                        $social = array_merge($social, $youtubeResponse->firstOrFail(function($channel) use ($social) { return $channel["id"] == $social["id"]; }));
                    break;
                case "t":
                    $social = array_merge($social = $twitch->firstOrFail(function($channel) use ($social) { return $channel["c"] == $social["c"]; }));
                    if(isset($twitchResponse)) {
                        $updatedViews = $twitchResponse->first(function($channel) use ($social) { return $channel["id"] == $social["id"]; });
                        if($updatedViews)
                            $social = array_merge($social, $updatedViews);
                    }
                    break;
                default:
                    break;
            }

            return [
                "t" => $social["t"],
                "c" => $social["c"],
                "d" => $social["d"] ?? $social["c"],
                "f" => !empty($social["f"]) ? $this->formatNumber($social["f"]) : "0",
                "v" => !empty($social["v"]) && $social["v"] != -1 ? $this->formatNumber($social["v"]) : "0",
            ];
        });
    }

    /**
     * It gets a token from twitch, and if it's not in the cache, it will cache it for the duration of
     * the token minus 60 seconds
     *
     * @param fromCache If true, the function will try to get the token from the cache. If it's not
     * there, it will request a new one.
     *
     * @return The access token for the twitch api
     */
    private function getTwitchToken($fromCache = true) {
        if($fromCache && Cache::has("twitch_auth")) {
            return Cache::get("twitch_auth");
        } else {
            $tokenResponse = Http::asForm()->post("https://id.twitch.tv/oauth2/token", [
                "client_id" => config("socials.twitch.client_id"),
                "client_secret" => config("socials.twitch.client_secret"),
                "grant_type" => "client_credentials"
            ]);

            $tokenResponse->throwUnlessStatus(200);

            $validator = Validator::make([
                "access_token" => $tokenResponse["access_token"],
                "expires_in" => $tokenResponse["expires_in"]
            ], [
                "access_token" => "required",
                "expires_in" => "required|numeric|min:60"
            ]);
            if($validator->fails())
                return throw new AuthorizationException("Failed to autzhorize agains twitch");

            Cache::put("twitch_auth", $tokenResponse["access_token"], $tokenResponse["expires_in"] - 60);
            return $tokenResponse["access_token"];
        }
    }

    /**
     * It gets the user id of a twitch user
     *
     * @param login The login of the user you want to get the id of
     * @param fromCache If you want to get the user id from cache or not.
     *
     * @return The user id of the twitch user.
     */
    private function getTwitchUserId($login, $fromCache = true) {
        if(!$fromCache)
            Cache::forget("twitch_user_id_".$login);

        return Cache::rememberForever("twitch_user_id_".$login, function() use ($login) {
            $response = Http::withHeaders([
                "Authorization" => "Bearer ".$this->getTwitchToken(),
                "Client-Id" => config("socials.twitch.client_id")
            ])->get("https://api.twitch.tv/helix/users?login=".$login);

            if($response->status() == 401) {
                $response = Http::withHeaders([
                    "Authorization" => "Bearer ".$this->getTwitchToken(true),
                    "Client-Id" => config("socials.twitch.client_id")
                ])->get("https://api.twitch.tv/helix/users?login=".$login);
            }

            $response->throwUnlessStatus(200);

            $validator = Validator::make($response->collect()->toArray(), [
                "data" => "required|array|min:1",
                "data.*.id" => "required"
            ]);
            if($validator->fails())
                return throw new AuthorizationException("Failed to retrive data from twitch");

            return $response["data"][0]["id"];
        });
    }

    /**
     * It gets the number of followers of a twitch user
     *
     * @param userId The user id of the user you want to get the follower count of.
     * @param fromCache If you want to get the data from cache or not.
     *
     * @return The number of followers the user has.
     */
    private function getTwitchFollower($userId, $fromCache = true) {
        if(empty($userId))
            return 0;

        if(!$fromCache)
            Cache::forget("twitch_user_follower_".$userId);

        return Cache::remember("twitch_user_follower_".$userId, 3600, function() use ($userId) {
            $response = Http::withHeaders([
                "Authorization" => "Bearer ".$this->getTwitchToken(),
                "Client-Id" => config("socials.twitch.client_id")
            ])->get("https://api.twitch.tv/helix/users/follows?first=1&to_id=".$userId);

            if($response->status() == 401) {
                $response = Http::withHeaders([
                    "Authorization" => "Bearer ".$this->getTwitchToken(true),
                    "Client-Id" => config("socials.twitch.client_id")
                ])->get("https://api.twitch.tv/helix/users/follows?first=1&to_id=".$userId);
            }

            $response->throwUnlessStatus(200);

            $validator = Validator::make([
                "total" => $response["total"]
            ], [
                "total" => "required|numeric"
            ]);
            if($validator->fails())
                return throw new AuthorizationException("Failed to retrive data from twitch");

            return $response["total"];
        });
    }

    /**
     * It takes an array of user ids, makes a request to the twitch api, and returns an array of user
     * ids and their viewer count
     *
     * @param array userIds An array of user ids to request the viewer count for.
     *
     * @return An array of user ids and their current viewer count.
     */
    private function requestTwitchViewers(array $userIds) {

        $url = "https://api.twitch.tv/helix/streams";

        $counter = 0;
        foreach($userIds as $userId) {
            if(!empty($userId)) {
                $url .= $counter == 0 ? "?user_id=".$userId : "&user_id=".$userId;
                $counter++;
            }
        }

        if($counter == 0)
            return [];

        $response = Http::withHeaders([
            "Authorization" => "Bearer ".$this->getTwitchToken(),
            "Client-Id" => config("socials.twitch.client_id")
        ])->get($url);

        if($response->status() == 401) {
            $response = Http::withHeaders([
                "Authorization" => "Bearer ".$this->getTwitchToken(true),
                "Client-Id" => config("socials.twitch.client_id")
            ])->get($url);
        }

        $response->throwUnlessStatus(200);

        $validator = Validator::make($response->collect()->toArray(), [
            "data" => "array",
            "data.*.viewer_count" => "required|numeric",
            "data.*.user_id" => "required",
        ]);
        if($validator->fails()) {
            return throw new AuthorizationException("Failed to retrive data from twitch");
        }

        foreach($userIds as $userId) {
            Cache::put("twitch_user_views_".$userId, -1, 60);
        }

        if(empty($response["data"]))
            return [];

        return collect($response["data"])->map(function($user) {

            Cache::put("twitch_user_views_".$user["user_id"], $user["viewer_count"], 60);

            return [
                "v" => $user["viewer_count"],
                "id" => $user["user_id"],
            ];
        });
    }

    /**
     * It takes an array of channel ids, makes a request to the youtube api, validates the response,
     * and returns an array of objects containing the channel id, view count, and follower count
     *
     * @param channelIds An array of channel ids to request stats for.
     *
     * @return An array of objects with the following properties:
     * - id
     * - v
     * - f
     */
    private function requestYoutubeChannelStats($channelIds) {

        $filteredIds = array_filter($channelIds, function($id) { return !empty($id); });
        if($filteredIds == 0)
            return [];

        $response = Http::get("https://www.googleapis.com/youtube/v3/channels?part=statistics&id=".join(",", $filteredIds)."&key=".config("socials.youtube.api_key"));

        $response->throwUnlessStatus(200);

        $validator = Validator::make($response->collect()->toArray(), [
            "items" => "required|array|size:".sizeof($channelIds),
            "items.*.id" => "required",
            "items.*.statistics.viewCount" => "required|numeric",
            "items.*.statistics.subscriberCount" => "required|numeric"
        ]);
        if($validator->fails())
            return throw new AuthorizationException("Failed to retrive data from youtube");

        return collect($response["items"])->map(function($item) {

            Cache::put("youtube_channel_views_".$item["id"], $item["statistics"]["viewCount"], 3600);
            Cache::put("youtube_channel_subs_".$item["id"], $item["statistics"]["subscriberCount"], 3600);

            return [
                "id" => $item["id"],
                "v" => $item["statistics"]["viewCount"],
                "f" => $item["statistics"]["subscriberCount"],
            ];
        });
    }

    /**
     * It gets the channel id from youtube, and caches it forever
     *
     * @param channelName The name of the channel you want to get the id for.
     * @param fromCache If set to true, the function will try to get the channel id from the cache. If
     * it's not there, it will make a request to the youtube api and cache the result.
     *
     * @return The channel ID of the channel name provided.
     */
    private function getYoutubeChannelId($channelName, $fromCache = true): string {
        if(!$fromCache)
            Cache::forget("youtube_channel_id_".$channelName);

        return Cache::rememberForever("youtube_channel_id_".$channelName, function() use ($channelName) {
            $response = Http::get("https://www.googleapis.com/youtube/v3/search?part=id&maxResults=1&q=".$channelName."&key=".config("socials.youtube.api_key"));

            $response->throwUnlessStatus(200);

            $validator = Validator::make($response->collect()->toArray(), [
                "items" => "required|array|min:1",
                "items.*.id.channelId" => "required"
            ]);
            if($validator->fails())
                return throw new AuthorizationException("Failed to retrive data from youtube");

            return $response["items"][0]["id"]["channelId"];
        });
    }

    /**
     * It requests the Instagram API for the user's ID, followers and following count
     *
     * @param username The username of the user you want to get the stats of.
     *
     * @return An array with the user's id, followers, and following.
     */
    private function requestInstagramStats($username) {
        $response = Http::withHeaders([
            "User-Agent" => "Mozilla/5.0 (iPhone; CPU iPhone OS 13_1_3 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 Instagram 118.0.0.25.121 (iPhone11,8; iOS 13_1_3; en_US; en-US; scale=2.00; 828x1792; 180988914)"
        ])->withCookies([
            "sessionid" => config("socials.instagram.session_id", "")
        ], "instagram.com")->get("https://i.instagram.com/api/v1/users/web_profile_info/?username=".$username);

        $response->throwUnlessStatus(200);

        $validator = Validator::make([
            "data" => $response["data"]
        ], [
            "data.user.id" => "required",
            "data.user.edge_followed_by.count" => "required|numeric",
            "data.user.edge_follow.count" => "required|numeric"
        ]);
        if($validator->fails())
            return throw new AuthorizationException("Failed to retrive data from twitch");

        Cache::forever("instagram_user_id_".$username, $response["data"]["user"]["id"]);
        Cache::put("instagram_user_follower_".$username, $response["data"]["user"]["edge_followed_by"]["count"], 3600);
        Cache::put("instagram_user_following_".$username, $response["data"]["user"]["edge_follow"]["count"], 3600);

        return [
            "id" => $response["data"]["user"]["id"],
            "f" => $response["data"]["user"]["edge_followed_by"]["count"],
            "v" => $response["data"]["user"]["edge_follow"]["count"],
        ];
    }

    /**
     * It takes a number and returns a string with the number formatted with a thousand separator
     *
     * @param number The number to format.
     *
     * @return string The number is being formatted with a period as the thousands separator and a
     * comma as the decimal separator.
     */
    private function formatNumber($number): string {
        try {
            return number_format($number, 0, ",", ".");
        } catch(\Exception) {
            return 0;
        }
    }
}
