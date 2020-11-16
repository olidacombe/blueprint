/*
  ==============================================================================

    blueprint_EcmascriptEngineTests.cpp
    Created: 26 Oct 2019 3:47:39pm

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"

class EcmascriptEngineEvaluateTest  : public juce::UnitTest
{
public:
    EcmascriptEngineEvaluateTest()
        : juce::UnitTest ("Testing script evaluation") {}

    void runTest() override
    {
        blueprint::EcmascriptEngine engine;

        beginTest ("Numbers");
        expect (5 == (int) engine.evaluate("2 + 3;"));
        expect (6 == (int) engine.evaluate("2 * 3;"));
        expect (4 == (int) engine.evaluate("Math.pow(2, 2);"));

        beginTest ("Strings");
        expect (juce::String("hey") == engine.evaluate("['h', 'e', 'y'].join('');").toString());
    }
};

class EcmascriptEngineNativeFunctionTest  : public juce::UnitTest
{
public:
    EcmascriptEngineNativeFunctionTest()
        : juce::UnitTest ("Testing native function interface") {}

    void runTest() override
    {
        blueprint::EcmascriptEngine engine;

        beginTest ("Global function");

        engine.registerNativeMethod("myMultiply", [](void* stash, const juce::var::NativeFunctionArgs& args) {
            EcmascriptEngineNativeFunctionTest* self = reinterpret_cast<EcmascriptEngineNativeFunctionTest*>(stash);

            jassert (self != nullptr);
            self->expect(args.numArguments == 2);

            int left = args.arguments[0];
            int right = args.arguments[1];

            return juce::var(left * right);
        }, (void *) this);

        expect (6 == (int) engine.evaluate("myMultiply(2, 3);"));
        expect (4 == (int) engine.evaluate("this.myMultiply(2, 2);"));

        beginTest ("Namespaced function");

        engine.registerNativeProperty("Blueprint", juce::JSON::parse("{}"));
        engine.registerNativeMethod("Blueprint", "squareIt", [](void* stash, const juce::var::NativeFunctionArgs& args) {
            EcmascriptEngineNativeFunctionTest* self = reinterpret_cast<EcmascriptEngineNativeFunctionTest*>(stash);

            jassert (self != nullptr);
            self->expect(args.numArguments == 1);

            int left = args.arguments[0];

            return juce::var(left * left);
        }, (void *) this);

        expect (4 == (int) engine.evaluate("Blueprint.squareIt(2);"));
        expect (9 == (int) engine.evaluate("this.Blueprint.squareIt(3);"));
    }
};

class EcmascriptEngineNativePropertyTest  : public juce::UnitTest
{
public:
    EcmascriptEngineNativePropertyTest()
        : juce::UnitTest ("Testing native property interface") {}

    void runTest() override
    {
        blueprint::EcmascriptEngine engine;

        beginTest ("Global property");
        engine.registerNativeProperty("BlueprintNative", juce::JSON::parse("{}"));
        engine.registerNativeProperty("DOUBLE_PI", 3.14159 * 2.0);
        engine.registerNativeProperty("PLUGIN_VERSION", "1.3.5");

        expect (3.14159 * 2.0 == (double) engine.evaluate("DOUBLE_PI;"));
        expect (juce::String("1.3.5") == engine.evaluate("PLUGIN_VERSION").toString());
        expect (engine.evaluate("BlueprintNative").isObject());

        beginTest ("Namespaced property");
        engine.registerNativeProperty("BlueprintNative", "Constants", juce::JSON::parse("{}"));
        engine.registerNativeProperty("BlueprintNative.Constants", "PLUGIN_VERSION", "1.3.5");
        engine.registerNativeProperty("BlueprintNative.Constants", "PLUGIN_NAME", "Temper");

        expect (juce::String("1.3.5") == engine.evaluate("BlueprintNative.Constants.PLUGIN_VERSION").toString());
        expect (juce::String("Temper") == engine.evaluate("BlueprintNative.Constants.PLUGIN_NAME").toString());

        auto constants = engine.evaluate("BlueprintNative.Constants");
        expect (constants.isObject());

        auto* obj = constants.getDynamicObject();
        expect (obj != nullptr);

        expect (obj->hasProperty("PLUGIN_VERSION"));
        expect (obj->hasProperty("PLUGIN_NAME"));
        expect (juce::String("1.3.5") == obj->getProperty("PLUGIN_VERSION").toString());
        expect (juce::String("Temper") == obj->getProperty("PLUGIN_NAME").toString());
    }
};

class EcmascriptEngineInvokeTest  : public juce::UnitTest
{
public:
    EcmascriptEngineInvokeTest()
        : juce::UnitTest ("Testing function invocation interface") {}

    void runTest() override
    {
        blueprint::EcmascriptEngine engine;

        beginTest ("Invoking builtins");
        expect (juce::String("*") == engine.invoke("String.fromCharCode", 42).toString());
        expect (1 == (int) engine.invoke("parseInt", 1.00031439, 10));
        expect (1 == (int) engine.invoke("Math.abs", -1));

        beginTest ("Invoking native methods");
        engine.registerNativeProperty("BlueprintNative", juce::JSON::parse("{}"));

        engine.registerNativeMethod("BlueprintNative", "squareIt", [](void* /* stash */, const juce::var::NativeFunctionArgs& args) {
            int left = args.arguments[0];
            return juce::var(left * left);
        });

        expect (4 == (int) engine.invoke("BlueprintNative.squareIt", 2));
        expect (9 == (int) engine.invoke("BlueprintNative.squareIt", 3));
    }
};

class EcmascriptEngineErrorHandlerTest  : public juce::UnitTest
{
public:
    EcmascriptEngineErrorHandlerTest()
        : juce::UnitTest ("Testing error handler interface") {}

    template <typename T, typename... Args>
    void testUncaughtError(T&& fn, Args... args)
    {
        blueprint::EcmascriptEngine engine;
        bool didThrow = false;

        try {
            std::invoke(fn, engine, std::forward<Args>(args)...);
        } catch (const std::runtime_error& e) {
            didThrow = true;
        }

        expect(didThrow);
    }

    template <typename T, typename... Args>
    void testCaughtError(T&& fn, Args... args)
    {
        blueprint::EcmascriptEngine engine;
        bool didThrow = false;

        engine.onUncaughtError = [&didThrow](const juce::String& msg, const juce::String& trace) {
            didThrow = true;
        };

        std::invoke(fn, engine, std::forward<Args>(args)...);
        expect(didThrow);
    }

    void runTest() override
    {
        beginTest("Parse errors");
        testUncaughtError(&blueprint::EcmascriptEngine::evaluate, "1 + 2 + ");
        testCaughtError(&blueprint::EcmascriptEngine::evaluate, "1 + 2 + ");
        testUncaughtError(&blueprint::EcmascriptEngine::invoke<int>, "Blueprint.1+", 1);
        testCaughtError(&blueprint::EcmascriptEngine::invoke<int>, "Blueprint.1+", 1);

        testUncaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeMethod("global[1 + 2 +]", "Noop", [](void *, const juce::var::NativeFunctionArgs&) {
                // Noop
                return juce::var::undefined();
            }, nullptr);
        });

        testCaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeMethod("global[1 + 2 +]", "Noop", [](void *, const juce::var::NativeFunctionArgs&) {
                // Noop
                return juce::var::undefined();
            }, nullptr);
        });

        testUncaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeProperty("global[1 + 2 +]", "Noop", 42);
        });

        testCaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeProperty("global[1 + 2 +]", "Noop", 42);
        });

        beginTest("Runtime errors");
        testUncaughtError(&blueprint::EcmascriptEngine::evaluate, "doesNotExist();");
        testCaughtError(&blueprint::EcmascriptEngine::evaluate, "doesNotExist();");
        testUncaughtError(&blueprint::EcmascriptEngine::invoke<int>, "Blueprint[doesNotExist()]", 1);
        testCaughtError(&blueprint::EcmascriptEngine::invoke<int>, "Blueprint[doesNotExist()]", 1);

        testUncaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeMethod("global[doesNotExist()]", "Noop", [](void *, const juce::var::NativeFunctionArgs&) {
                // Noop
                return juce::var::undefined();
            }, nullptr);
        });

        testCaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeMethod("global[doesNotExist()]", "Noop", [](void *, const juce::var::NativeFunctionArgs&) {
                // Noop
                return juce::var::undefined();
            }, nullptr);
        });

        testUncaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeProperty("global[doesNotExist()]", "Noop", 42);
        });

        testCaughtError([](blueprint::EcmascriptEngine& engine) {
            engine.registerNativeProperty("global[doesNotExist()]", "Noop", 42);
        });
    }
};

// Create static instances to register it with the array run by
// UnitTestRunner::runAllTests()
static EcmascriptEngineEvaluateTest evTest;
static EcmascriptEngineNativeFunctionTest fnTest;
static EcmascriptEngineNativePropertyTest propTest;
static EcmascriptEngineInvokeTest invTest;
static EcmascriptEngineErrorHandlerTest errTest;
