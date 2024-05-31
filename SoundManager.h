#pragma once
#include <SFML/Audio.hpp>
#include <functional>
#include <vector>


class SoundManager {
public:
    int masterVolume { 50 };
    int musicVolume { 0 }; // if you alter these, change the setting in the Menu constructor as well so it is matched from the start (not a big deal if you don't, but it will be a visual bug)
    int soundVolume { 50 };

    SoundManager()
    {

        /*

            TO ADD A SOUND:

                1. Add the sound to the resources/sounds folder
                2. Load the sound in the constructor right below this comment
                3. Add a buffer for the sound AT THE VERY BOTTOM OF THIS FILE
                4. add it to the switch statement in PlaySound(std::string name)


                you can then play the sound by calling:

                >        soundManager.RegisterEventCallback([&]() { soundManager.PlaySound("pickUp"); });

        */
        buffer.loadFromFile("resources/sounds/drillShort1.wav");
        buffer2.loadFromFile("resources/sounds/drillWood.wav");
        buffer3.loadFromFile("resources/sounds/pickUp.wav");
        buffer4.loadFromFile("resources/sounds/bambooWoosh.wav");
        buffer5.loadFromFile("resources/sounds/longWoosh.wav");

        // Initialize the sound pool
        for (int i = 0; i < MaxSounds; ++i) {
            sf::Sound sound;
            sound.setVolume(static_cast<float>(masterVolume));
            sounds.push_back(sound);
        }

        // Initialize the background music
        backgroundMusic.openFromFile("resources/music/lunarDrive.ogg");
        backgroundMusic.setVolume(static_cast<float>(masterVolume * musicVolume / 100.0));
        backgroundMusic.setLoop(true);
        PlayBackgroundMusic();
    }

    void PlaySound(int index)
    {
        if (index >= 0 && index < MaxSounds) {
            sounds[index].play();
            sounds[index].setVolume(static_cast<float>(masterVolume * soundVolume / 100.0));
        }
    }

    void SetMasterVolume(int vol)
    {
        masterVolume = vol;
        backgroundMusic.setVolume(static_cast<float>(masterVolume * musicVolume / 100.0));
        for (auto& sound : sounds) {
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

    void PlaySound(std::string name)
    {
        sf::SoundBuffer* soundBuffer = nullptr;

        if (name == "drill") {
            soundBuffer = &buffer;
        } else if (name == "drillWood") {
            soundBuffer = &buffer2;
        } else if (name == "pickUp") {
            soundBuffer = &buffer3;
        } else if (name == "bambooWoosh") {
            soundBuffer = &buffer4;
        } else if (name == "longWoosh") {
            soundBuffer = &buffer5;
        }

        if (soundBuffer != nullptr) {
            // Find an available sound in the pool
            for (auto& sound : sounds) {
                if (sound.getStatus() == sf::Sound::Stopped) {
                    sound.setBuffer(*soundBuffer);
                    sound.play();
                    sound.setVolume(static_cast<float>(masterVolume * soundVolume / 100.0));
                    break;
                }
            }
        } else {
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
        for (auto& callback : eventCallbacks) {
            callback();
        }

        // Clear the event callbacks after handling them
        eventCallbacks.clear();
    }

    // Function to update the sound manager
    void Update()
    {
        HandleEvent();
    }

    // Function to register event callbacks
    void RegisterEventCallback(std::function<void()> callback)
    {
        eventCallbacks.push_back(callback);
    }

private:
    sf::SoundBuffer buffer; // drill
    sf::SoundBuffer buffer2; // drillWood
    sf::SoundBuffer buffer3; // pickUp
    sf::SoundBuffer buffer4; // bambooWoosh
    sf::SoundBuffer buffer5; // longWoosh

    std::vector<sf::Sound> sounds;
    std::vector<std::function<void()>> eventCallbacks;

    sf::Music backgroundMusic;

    // Maximum number of sounds that can play simultaneously
    static const int MaxSounds = 20; // Adjust the number as needed
};

