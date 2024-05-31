#pragma once
#include <SFML/Audio.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class SoundManager
{
  public:
    int masterVolume{50};
    int musicVolume{10};
    int soundVolume{50};
    int masterVolumeBuffer{50};
    int musicVolumeBuffer{10};
    int soundVolumeBuffer{50};
    glm::vec3 playerPosition = glm::vec3(0);
    int maxDistance = 50;

    SoundManager()
    {
        std::cout << "Initialising sound manager..." << std::endl;
        buffer.loadFromFile("res/sounds/blipSelect.wav");
        buffer2.loadFromFile("res/sounds/stoneImpact.wav");
        buffer3.loadFromFile("res/sounds/stoneSlide2.wav");
        std::cout << "Loaded sounds into buffers!" << std::endl;

        for (int i = 0; i < MaxSounds; ++i)
        {
            sf::Sound sound;
            sound.setVolume(static_cast<float>(masterVolume));
            sound.setRelativeToListener(false);
            sounds.push_back(sound);
        }
        std::cout << "Created vector of Sounds" << std::endl;

        backgroundMusic.openFromFile("res/sounds/lunarDrive.ogg");
        backgroundMusic.setVolume(static_cast<float>(masterVolume * musicVolume / 100.0));
        backgroundMusic.setLoop(true);
        std::cout << "Starting background music..." << std::endl;
        PlayBackgroundMusic();
        std::cout << "SUCCESS: Sound manager initialised!" << std::endl;
    }

    void PlaySound(const std::string &name, const glm::vec3 &position)
    {
        if (playingSounds[name])
        {
            return;
        }

        sf::SoundBuffer *soundBuffer = GetBufferForName(name);

        if (soundBuffer != nullptr)
        {
            for (auto &sound : sounds)
            {
                if (sound.getStatus() == sf::Sound::Stopped)
                {
                    sound.setBuffer(*soundBuffer);
                    sound.setPosition(position.x, position.y, position.z);
                    sound.setAttenuation(attenuation);
                    sound.setMinDistance(minDistance);

                    // Calculate distance between player and sound source
                    glm::vec3 soundDirection =
                        glm::vec3(sound.getPosition().x, sound.getPosition().y, sound.getPosition().z) - playerPosition;
                    float distance = glm::length(soundDirection);

                    // Adjust volume based on distance (larger distance = quieter)
                    float volume = static_cast<float>(masterVolume * soundVolume / 100.0f) *
                                   (1.0f - distance / maxDistance); // Adjust maxDistance for falloff range

                    sound.setVolume(std::max(volume, 0.0f)); // Ensure volume doesn't go negative
                    sound.play();
                    playingSounds[name] = true;
                    soundPositions[&sound] = position;
                    break;
                }
            }
        }
        else
        {
            throw std::runtime_error("Sound not found");
        }
    }

    void PlayBackgroundMusic()
    {
        backgroundMusic.play();
    }

    void StopBackgroundMusic()
    {
        backgroundMusic.stop();
    }

    void SetBackgroundMusicVolume(int volume)
    {
        musicVolume = volume;
        backgroundMusic.setVolume(static_cast<float>(masterVolume * musicVolume / 100.0));
    }

    void HandleEvent()
    {
        for (auto &callback : eventCallbacks)
        {
            callback();
        }

        eventCallbacks.clear();
        scheduledSounds.clear();
    }

    void Update(const glm::vec3 &playerPosition, const glm::vec3 &playerDirection)
    {
        this->playerPosition = playerPosition;
        HandleEvent();
        CleanupStoppedSounds();
        sf::Listener::setPosition(playerPosition.x, playerPosition.y, playerPosition.z);
        sf::Listener::setDirection(playerDirection.x, playerDirection.y, playerDirection.z);

        if (musicVolume != musicVolumeBuffer)
        {
            musicVolumeBuffer = musicVolume;
            SetBackgroundMusicVolume(musicVolume);
        }
    }

    void RegisterEventCallback(std::function<void()> callback)
    {
        eventCallbacks.push_back(callback);
    }

    void RegisterSoundEvent(const std::string &name, const glm::vec3 &position)
    {
        if (scheduledSounds.find(name) == scheduledSounds.end())
        {
            scheduledSounds.insert(name);
            RegisterEventCallback([&, name, position]() { PlaySound(name, position); });
        }
    }

    void SetMasterVolume(int vol)
    {
        masterVolume = vol;
        backgroundMusic.setVolume(static_cast<float>(masterVolume * musicVolume / 100.0));
        for (auto &sound : sounds)
        {
            sound.setVolume(static_cast<float>(masterVolume * soundVolume / 100.0));
        }
    }

    void SetSoundVolume(int vol)
    {
        soundVolume = vol;
    }

    void SetMusicVolume(int vol)
    {
        musicVolume = vol;
        backgroundMusic.setVolume(static_cast<float>(masterVolume * musicVolume / 100.0));
    }

  private:
    sf::SoundBuffer buffer;  // select
    sf::SoundBuffer buffer2; // impact
    sf::SoundBuffer buffer3; // slide

    std::vector<sf::Sound> sounds;
    std::vector<std::function<void()>> eventCallbacks;
    std::unordered_set<std::string> scheduledSounds;

    sf::Music backgroundMusic;

    std::unordered_map<std::string, bool> playingSounds;
    std::unordered_map<sf::Sound *, glm::vec3> soundPositions;

    static const int MaxSounds = 20;
    const float attenuation = 5.0f;  // Higher attenuation for noticeable falloff
    const float minDistance = 10.0f; // Smaller min distance for closer falloff

    void CleanupStoppedSounds()
    {
        for (auto &sound : sounds)
        {
            if (sound.getStatus() == sf::Sound::Stopped)
            {
                for (auto &entry : playingSounds)
                {
                    if (sound.getBuffer() == GetBufferForName(entry.first))
                    {
                        entry.second = false;
                        soundPositions.erase(&sound);
                        break;
                    }
                }
            }
        }
    }

    sf::SoundBuffer *GetBufferForName(const std::string &name)
    {
        if (name == "selectSound")
            return &buffer;
        else if (name == "impactSound")
            return &buffer2;
        else if (name == "slideSound")
            return &buffer3;
        else
            return nullptr;
    }
};
