#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    File sourceDir = File(__FILE__).getParentDirectory();
    File bundle = sourceDir.getChildFile("jsui/build/js/main.js");

    // Sanity check
    jassert (bundle.existsAsFile());

    addAndMakeVisible(appRoot);
    appRoot.evaluate(bundle);

    // setResizable(true, true);
    // setResizeLimits(400, 240, 400 * 2, 240 * 2);
    // getConstrainer()->setFixedAspectRatio(400.0 / 240.0);
    // setSize (400, 240);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g) {}

void MainComponent::resized()
{
  appRoot.setBounds(getLocalBounds());
}
