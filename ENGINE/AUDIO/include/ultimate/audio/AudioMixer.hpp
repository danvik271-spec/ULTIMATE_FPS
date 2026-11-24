#pragma once
#include "ultimate/math/Vec.hpp"
#include <string>
#include <vector>

namespace ultimate::audio {
struct AudioCue { std::string assetPath; math::Vec3 position{}; float gain{1.0F}; bool spatial{true}; };
struct MixedCue { std::string assetPath; float gain{0.0F}; float pan{0.0F}; };
class AudioMixer {
public:
    void setListener(math::Vec3 position, math::Vec3 right) noexcept;
    void submit(AudioCue cue);
    [[nodiscard]] std::vector<MixedCue> mixFrame();
    void setMasterGain(float gain) noexcept;
private:
    math::Vec3 listener_{};
    math::Vec3 listenerRight_{1.0F, 0.0F, 0.0F};
    float masterGain_{1.0F};
    std::vector<AudioCue> pending_;
};
}
