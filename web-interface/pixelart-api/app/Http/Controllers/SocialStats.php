<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class SocialStats extends Controller
{
    public function get() {
        return response()->json(["status" => "up"]);
    }
}
