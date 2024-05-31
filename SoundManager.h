#pragma once
#include <SFML/Audio.hpp>
#include <functional>
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

    SoundManager()
    {
        std::cout << "Initialising sound manager..." << std::endl;
        buffer.loadFromFile("res/sounds/blipSelect.wav");
        buffer2.loadFromFile("res/sounds/stoneImpact.wav");
        buffer3.loadFromFile("res/sounds/stoneSlide.wav");
        std::cout << "Loaded sounds into buffers!" << std::endl;

        for (int i = 0; i < MaxSounds; ++i)
        {
            sf::Sound sound;
            sound.setVolume(static_cast<float>(masterVolume));
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

    void PlaySound(int index)
    {
        if (index >= 0 && index < MaxSounds)
        {
            sounds[index].play();
            sounds[index].setVolume(static_cast<float>(masterVolume * soundVolume / 100.0));
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

    void PlaySound(const std::string &name)
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
                    sound.play();
                    sound.setVolume(static_cast<float>(masterVolume * soundVolume / 100.0));
                    playingSounds[name] = true;
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

    void Update()
    {
        HandleEvent();
        CleanupStoppedSounds();
    }

    void RegisterEventCallback(std::function<void()> callback)
    {
        eventCallbacks.push_back(callback);
    }

    void RegisterSoundEvent(const std::string &name)
    {
        if (scheduledSounds.find(name) == scheduledSounds.end())
        {
            scheduledSounds.insert(name);
            RegisterEventCallback([&, name]() { PlaySound(name); });
        }
    }
    std::unordered_map<std::string, bool> playingSounds;

  private:
    sf::SoundBuffer buffer;  // select
    sf::SoundBuffer buffer2; // impact
    sf::SoundBuffer buffer3; // slide

    std::vector<sf::Sound> sounds;
    std::vector<std::function<void()>> eventCallbacks;
    std::unordered_set<std::string> scheduledSounds;

    sf::Music backgroundMusic;

    static const int MaxSounds = 20;

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
