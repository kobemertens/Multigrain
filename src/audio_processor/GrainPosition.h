//
// Created by kobe on 19/11/2021.
//

#pragma once

/**
 * Represents the playback position of a grain in a buffer
 */
struct GrainPosition
{
    double leftPosition;
    double rightPosition;

    template<typename FloatType>
    GrainPosition fmod(FloatType x) const noexcept
    {
        return {std::fmod(leftPosition, x), std::fmod(rightPosition, x)};
    }

    GrainPosition operator+(GrainPosition other)
    {
        return {leftPosition + other.leftPosition, rightPosition + other.rightPosition};
    }


    GrainPosition operator+(double x)
    {
        return {leftPosition + x, rightPosition + x};
    }
};