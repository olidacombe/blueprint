#pragma once

template <typename Error>
struct TimeoutFunctionManager : private juce::MultiTimer
{
    ~TimeoutFunctionManager() override {
        for(const auto &[id, timer] : timeoutFunctions)
            stopTimer(id);
    }

    juce::var clearTimeout(const int id)
    {
        stopTimer(id);
        const auto f = timeoutFunctions.find(id);
        if(f != timeoutFunctions.cend())
            timeoutFunctions.erase(f);
        return juce::var();
    }

    int newTimeout(const juce::var::NativeFunction f, const int timeoutMillis, const std::vector<juce::var>&& args, const bool repeats=false)
    {
        static int nextId = 0;
        timeoutFunctions.emplace(nextId, TimeoutFunction(f, std::move(args), repeats));
        startTimer(nextId, timeoutMillis);
        return nextId++;
    }

    void timerCallback(int id) override
    {
        const auto f = timeoutFunctions.find(id);
        if(f != timeoutFunctions.cend())
        {
            const auto cb = f->second;
            std::invoke(cb.f, juce::var::NativeFunctionArgs(juce::var(), cb.args.data(), static_cast<int>(cb.args.size())));
            if(!cb.repeats)
            {
                stopTimer(id);
                timeoutFunctions.erase(f);
            }
        }
    }

    private:
        struct TimeoutFunction
        {
            TimeoutFunction(const juce::var::NativeFunction _f, const std::vector<juce::var> &&_args, const bool _repeats=false)
            : f(_f), args(std::move(_args)), repeats(_repeats) {}

            const juce::var::NativeFunction f;
            std::vector<juce::var> args;
            const bool repeats;
        };

        std::map<int, TimeoutFunction> timeoutFunctions;
    };