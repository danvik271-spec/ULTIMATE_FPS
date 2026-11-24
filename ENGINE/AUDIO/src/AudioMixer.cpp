#include "ultimate/audio/AudioMixer.hpp"
#include <algorithm>

namespace ultimate::audio {
void AudioMixer::setListener(const math::Vec3 position, const math::Vec3 right) noexcept {
    listener_ = position; listenerRight_ = math::normalized(right);
}
void AudioMixer::submit(AudioCue cue) { pending_.push_back(std::move(cue)); }
void AudioMixer::setMasterGain(const float gain) noexcept { masterGain_ = std::clamp(gain, 0.0F, 1.0F); }
std::vector<MixedCue> AudioMixer::mixFrame() {
    std::vector<MixedCue> output; output.reserve(pending_.size());
    for (const auto& cue : pending_) {
        const auto relative = cue.position - listener_;
        const float distance = math::length(relative);
        const float attenuation = cue.spatial ? 1.0F / (1.0F + 0.15F * distance * distance) : 1.0F;
        const float pan = cue.spatial ? std::clamp(math::dot(math::normalized(relative), listenerRight_), -1.0F, 1.0F) : 0.0F;
        output.push_back({cue.assetPath, std::clamp(cue.gain, 0.0F, 1.0F) * masterGain_ * attenuation, pan});
    }
    pending_.clear();
    return output;
}
}
