#include "FrameRate.h"
#include <stdexcept>

FrameRateLimiter::FrameRateLimiter(double target_fps)
    : current_target_fps_(target_fps)
{
    if (target_fps <= 0.0) {
        throw std::invalid_argument("Target FPS must be greater than 0");
    }

    time_between_frames_ = std::chrono::nanoseconds(static_cast<long long>(1e9 / target_fps));
    next_frame_time_ = std::chrono::steady_clock::now();
}

void FrameRateLimiter::SetTargetFPS(double target_fps) {
    if (target_fps <= 0.0) {
        throw std::invalid_argument("Target FPS must be greater than 0");
    }

    current_target_fps_ = target_fps;
    time_between_frames_ = std::chrono::nanoseconds(static_cast<long long>(1e9 / target_fps));
}

void FrameRateLimiter::Sleep() {
    // Add the frame duration to our target time
    next_frame_time_ += time_between_frames_;

    // Sleep until that time point
    std::this_thread::sleep_until(next_frame_time_);
}

void FrameRateLimiter::Reset() {
    next_frame_time_ = std::chrono::steady_clock::now();
}

double FrameRateLimiter::GetTargetFPS() const {
    return current_target_fps_;
}