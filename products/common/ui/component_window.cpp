#include "products/common/ui/component_window.h"

ComponentWindow::ComponentWindow(Component* associatedComponent,
                                 bool closeOnClick)
: DocumentWindow("",Colours::lightgrey,DocumentWindow::closeButton,true),
  _component(associatedComponent)
{
  // Use native title bar from OS
  setUsingNativeTitleBar(true);
  
  _base_colour = juce::Colour (0xffb82c2f);
  
  setContentOwned(_component, true);
  
  setSize(_component->getWidth(),
          _component->getHeight());

  this->setVisible(true);
}

ComponentWindow::~ComponentWindow()
{
  _component = nullptr;
}

void ComponentWindow::closeButtonPressed()
{
  JUCEApplication::quit();
}