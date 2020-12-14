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

    int newTimeout(const juce::var::NativeFunctionArgs& args, const bool repeats=false)
    {
        static int nextId = 0;
        timeoutFunctions.emplace(nextId, TimeoutFunction(args, repeats));
        const int timeoutMillis = *(args.arguments + 1);
        startTimer(nextId, timeoutMillis);
        return nextId++;
    }

    int newInterval(const juce::var::NativeFunctionArgs& args)
    {
        return newTimeout(args, true);
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
            TimeoutFunction(const juce::var::NativeFunctionArgs& _args, const bool _repeats=false)
            : f(_args.arguments->getNativeFunction()), repeats(_repeats)
            {
                args.reserve(_args.numArguments - 2);
                for(int i = 2; i < _args.numArguments; i++)
                    args.push_back(*(_args.arguments + i));
            }

            const juce::var::NativeFunction f;
            std::vector<juce::var> args;
            const bool repeats;
        };

        std::map<int, TimeoutFunction> timeoutFunctions;
    };