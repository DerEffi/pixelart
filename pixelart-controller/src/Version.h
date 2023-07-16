#pragma once

#include <Arduino.h>

// Define a structure to compare two version.
// This program is working upto four version-tags
//
// Let's say version is: 1.2.3.4
// major = 1
// minor = 2
// revision = 3
// build = 4
//
// Here, major, minor, revision and build are version-tags
//
// This program is working upto four version-tags.
// This code has been tested with various type of inputs including
// less than four version tags.
//
// LIMITATION: VERSION TAGS MUST BE SEPARATED BY DOT.
// 1.2.3.4 -- VALID INPUT VERSION
// 1-2-3-4 -- INVALID INPUT VERSION
// 1.2.3-4 -- INVALID INPUT VERSION

struct Version
{
    private:
    // Define four member variables major, minor, revision and build.
    // Here, we are saying it as version-tag
    int major, minor, revision, build;

    public:
    // Parametarized constructor. Pass string to it and it will
    // extract version-tag from it.
    //
    // Use initializer list to assign version-tag variables
    // Assign it by zero, otherwise std::scanf() will assign
    // garbage value to the versio-tag, if number of version-tag
    // will be less than four.
    Version(const String& version)
        : major(0), minor(0), revision(0), build(0)
    {
        std::sscanf(version.c_str(), "%d.%d.%d.%d", &major, &minor, &revision, &build);

        // version-tag must be >=0, if it is less than zero, then make it zero.
        if (major < 0) major = 0;
        if (minor < 0) minor = 0;
        if (revision < 0) revision = 0;
        if (build < 0) build = 0;
    }

    // Overload greater than(>) operator to compare two version objects
    bool operator > (const Version& other)
    {
        // Start comparing version tag from left most.
        // There are three relation between two number. It could be >, < or ==
        // While comparing the version tag, if they are equal, then move to the
        // next version-tag. If it could be greater then return true otherwise
        // return false.

        // Compare major version-tag
        if (major > other.major)
            return true;
        else if (major < other.major)
            return false;

        // Compare moinor versio-tag
        // If control came here it means that above version-tag(s) are equal
        if (minor > other.minor)
            return true;
        else if (minor < other.minor)
            return false;

        // Compare revision versio-tag
        // If control came here it means that above version-tag(s) are equal
        if (revision > other.revision)
            return true;
        else if(revision < other.revision)
            return false;

        // Compare build versio-tag
        // If control came here it means that above version-tag(s) are equal
        if (build > other.build)
            return true;
        else if(build < other.build)
            return false;

        return false;
    }

    // Overload smaller than(<) operator to compare two version objects
    bool operator < (const Version& other)
    {
        // Start comparing version tag from left most.
        // There are three relation between two number. It could be >, < or ==
        // While comparing the version tag, if they are equal, then move to the
        // next version-tag. If it could be greater then return true otherwise
        // return false.

        // Compare major version-tag
        if (major < other.major)
            return true;
        else if (major > other.major)
            return false;

        // Compare moinor versio-tag
        // If control came here it means that above version-tag(s) are equal
        if (minor < other.minor)
            return true;
        else if (minor > other.minor)
            return false;

        // Compare revision versio-tag
        // If control came here it means that above version-tag(s) are equal
        if (revision < other.revision)
            return true;
        else if(revision > other.revision)
            return false;

        // Compare build versio-tag
        // If control came here it means that above version-tag(s) are equal
        if (build < other.build)
            return true;
        else if(build > other.build)
            return false;

        return false;
    }

    // Overload equal to(==) operator to compare two version
    bool operator == (const Version& other)
    {
        return major == other.major 
            && minor == other.minor 
            && revision == other.revision 
            && build == other.build;
    }

    // Overload not equal to(!=) operator to compare two version
    bool operator != (const Version& other)
    {
        return major != other.major 
            || minor != other.minor 
            || revision != other.revision 
            || build != other.build;
    }

    String toString() {
        return String((std::to_string(major) + '.' + std::to_string(minor) + '.' + std::to_string(revision) + '.' + std::to_string(build)).c_str());
    }
};