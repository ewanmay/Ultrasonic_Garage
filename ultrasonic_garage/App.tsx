import React, { Component } from 'react';
import { Platform, StyleSheet, Text, View } from 'react-native';
//Screen
import GarageScreen from './src/features/garage/containers/garage-screen-container'

//Redux
import reducers from './src/ducks/reducers'
import ReduxThunk from "redux-thunk";
import { Provider } from 'react-redux';
import { createStore, applyMiddleware } from "redux";

let store = createStore(reducers, {}, applyMiddleware(ReduxThunk));
export default class App extends Component {


  render() {
    return (
    <Provider store={store}>
      <GarageScreen />
    </Provider>
    );
  }
}