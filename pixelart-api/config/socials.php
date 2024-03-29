<?php

return [
    "twitch" => [
        "client_id" => env("TWITCH_CLIENT_ID"),
        "client_secret" => env("TWITCH_CLIENT_SECRET"),
    ],
    "youtube" => [
        "api_key" => env("YOUTUBE_API_KEY")
    ],
    "instagram" => [
        "username" => env("INSTAGRAM_USERNAME"),
        "password" => env("INSTAGRAM_PASSWORD")
    ]
];
