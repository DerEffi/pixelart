<?php

use App\Http\Controllers\SocialStats;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;

/*
|--------------------------------------------------------------------------
| API Routes
|--------------------------------------------------------------------------
|
| Here is where you can register API routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| is assigned the "api" middleware group. Enjoy building your API!
|
*/

Route::middleware("apikey")->group(function() {
    Route::get("/socials", [SocialStats::class, "get"]);
    Route::post("/socials", [SocialStats::class, "post"]);
});
