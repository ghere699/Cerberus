#pragma once

#include <chrono>
#include <thread>

class FrameRateLimiter {
public:
    /**
     * Construct a new Frame Rate Limiter
     * target_fps must be > 0
     */
    explicit FrameRateLimiter(double target_fps);

    void SetTargetFPS(double target_fps);

    void Sleep();

    void Reset();

    double GetTargetFPS() const;

private:
    std::chrono::nanoseconds time_between_frames_;
    std::chrono::steady_clock::time_point next_frame_time_;
    double current_target_fps_;
};