# <p align="center">Pixelart - Socials API</p>

## About

This Laravel App acts as service for the ESP32 to receive data from youtube, twitch and instagram. The authentication against this platforms is completely handled by this App, because of the limitations of the microcontroller.

## Installation

1. Install php on your system. Easiest with a server package like XAMPP or WAMPP also including a database server for later usage/debugging
1. Install php [composer](https://getcomposer.org/) on your system
1. Install package dependencies by executing `composer install` in the app directory
1. Copy the `.env.example` to `.env` and customize the environment settings
    - Add a database connection
    - Add credentials for a [Twitch App](https://dev.twitch.tv/console/extensions/create)
    - Add credentials for a Youtube/GoogleAPIs App. You need to add the capabilities of [YouTube Data API v3](https://developers.google.com/youtube/v3/getting-started) to your project in the google cloud console.
    - Add credentials for an instagram user. (No app, can be any user with permissions to view the account you want to display)
1. Generate a new encryption key with `php artisan key:generate`
1. Add an API key for authentication in the database by either manually adding it in the table or using the DatabaaseSeeder: `php artisan db:seed`

## Deployment

You can host the app like any other [Laravel App](https://laravel.com/docs/9.x/deployment) by simply copying the files in the hosting directory of your webserver by i.e.
    - Using a webserver like Apache or Nginx directly
    - Using a webserver package like XAMPP or WAMPP
    - Deploying on a homeserver like a Raspberry Pi
    - Deploying on a webserver/shared webhosting 

# Usage

You can find a [openapi documentation](../docs/API/openapi.yml) and a [Postman Collection](../docs/API/Pixelart.postman_collection.json) about the available endpoints under [/docs/API](../docs/API/openapi.yml).
Make sure to add the `Authorization` Header with the API key from the Installation before makin any calls.
