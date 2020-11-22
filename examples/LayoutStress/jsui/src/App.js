import React, { Component } from 'react';
import {
  Text,
  View,
} from 'react-juce';

class App extends Component {
  constructor(props) {
    super(props);

    this.state = {};
  }

  render() {

    return (
      <View {...styles.container}>
        <Text>Lol</Text>
      </View>
    );
  }
}

const styles = {
  container: {
    width: '100%',
    height: '100%',
    'background-color': 'ffffffff',
    'justify-content': 'center',
    'align-items': 'center',
  },
};

export default App;
