<?php

namespace App\Http\Controllers;

use Illuminate\Auth\Access\AuthorizationException;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Cache;
use Illuminate\Support\Facades\Http;
use Spatie\FlareClient\Http\Exceptions\InvalidData;
use Illuminate\Support\Facades\Validator;

class SocialStats extends Controller
{
    public function get(Request $request) {

        try {
            $base64_socials = $request->get('socials');
            if(empty($base64_socials))
                throw new InvalidData();

            $json_socials = base64_decode($base64_socials);
            if(empty($json_socials))
                throw new InvalidData();
            $socials = json_decode($json_socials, true);
            if(empty($socials))
                throw new InvalidData();

            $validator = Validator::make($socials, [
                "*.t" => "required|in:y,t",
                "*.c" => "required|min:4|max:60"
            ]);
            if($validator->fails())
                return response()->json(["message" => "wrong request format. Instead use [{'t': 'y' | 't', 'c': %channelname%}]"], 400);

            $socials = collect($socials)->map(function($social) {

                try {
                    switch($social["t"]) {
                        case "t":
                            $social["f"] = (int)$this->getTwitchFollower($this->getTwitchUserId($social["c"]));
                            $social["v"] = (int)$this->getTwitchViewer($this->getTwitchUserId($social["c"]));
                            break;
                        case "y":
                            $ytStats = $this->getYoutubeChannel($social["c"]);
                            $social["f"] = (int)$ytStats["subs"];
                            $social["v"] = (int)$ytStats["views"];
                            break;
                    }
                } catch(\Exception $e) {
                    $social["f"] = 0;
                    $social["v"] = 0;
                    return $social;
                }

                return $social;
            });

            return response()->json($socials);
        } catch(\Exception $e) {
            dd($e);
            return response()->json(["message" => "get parameter 'socials' is required to have a valid base64 encoded json array of requested channel data"], 400);
        }
    }


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

    private function getTwitchUserId($login, $fromCache = true) {
        if(!$fromCache)
            Cache::forget("twitch_channel_".$login);

        return Cache::rememberForever("twitch_channel_".$login, function() use ($login) {
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

            $validator = Validator::make([
                "id" => $response["data"][0]["id"]
            ], [
                "id" => "required"
            ]);
            if($validator->fails())
                return throw new AuthorizationException("Failed to retrive data from twitch");

            return $response["data"][0]["id"];
        });
    }

    private function getTwitchFollower($userId, $fromCache = true) {
        if(!$fromCache)
            Cache::forget("twitch_user_".$userId."_follower");

        return Cache::remember("twitch_user_".$userId."_follower", 60, function() use ($userId) {
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

    private function getTwitchViewer($userId, $fromCache = true) {
        if(!$fromCache)
            Cache::forget("twitch_user_".$userId."_viewer");

        return Cache::remember("twitch_user_".$userId."_viewer", 30, function() use ($userId) {
            $response = Http::withHeaders([
                "Authorization" => "Bearer ".$this->getTwitchToken(),
                "Client-Id" => config("socials.twitch.client_id")
            ])->get("https://api.twitch.tv/helix/streams?user_id=".$userId);

            if($response->status() == 401) {
                $response = Http::withHeaders([
                    "Authorization" => "Bearer ".$this->getTwitchToken(true),
                    "Client-Id" => config("socials.twitch.client_id")
                ])->get("https://api.twitch.tv/helix/streams?user_id=".$userId);
            }

            $response->throwUnlessStatus(200);

            if(empty($response["data"]))
                return 0;

            $validator = Validator::make([
                "viewer_count" => $response["data"][0]["viewer_count"]
            ], [
                "viewer_count" => "required|numeric"
            ]);
            if($validator->fails())
                return throw new AuthorizationException("Failed to retrive data from twitch");

            return $response["data"][0]["viewer_count"];
        });
    }

    private function getYoutubeChannel($username, $fromCache = true) {
        if($fromCache && Cache::has("youtube_user_".$username."_views")  && Cache::has("youtube_user_".$username."_subs")) {
            return [
                "views" => Cache::get("youtube_user_".$username."_views"),
                "subs" => Cache::get("youtube_user_".$username."_subs"),
            ];
        } else {

            $response = Http::get("https://www.googleapis.com/youtube/v3/channels?part=statistics&forUsername=".$username."&key=".config("socials.youtube.api_key"));

            $response->throwUnlessStatus(200);

            if(empty($response["items"]) || empty($response["items"][0]["statistics"]))
                return [
                    "views" => 0,
                    "subs" => 0,
                ];

            $validator = Validator::make($response["items"][0]["statistics"], [
                "viewCount" => "required|numeric",
                "subscriberCount" => "required|numeric"
            ]);
            if($validator->fails())
                return throw new AuthorizationException("Failed to retrive data from youtube");

            Cache::put("youtube_user_".$username."_views", $response["items"][0]["statistics"]["viewCount"], 3600);
            Cache::put("youtube_user_".$username."_subs", $response["items"][0]["statistics"]["subscriberCount"], 3600);

            return [
                "views" => $response["items"][0]["statistics"]["viewCount"],
                "subs" => $response["items"][0]["statistics"]["subscriberCount"],
            ];
        }
    }
}
