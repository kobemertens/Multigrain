//
// Created by kobe on 19/11/2021.
//

#pragma once

/**
 * Represents the playback position of a grain in a buffer
 */
template<typename T>
struct Pair
{
    T l;
    T r;

    Pair fmod(T x) const noexcept
    {
        return {std::fmod(l, x), std::fmod(r, x)};
    }

    Pair operator+(Pair other)
    {
        return {l + other.l, r + other.r};
    }

    Pair operator+(T x)
    {
        return {l + x, r + x};
    }
};