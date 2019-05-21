import React, { Component } from 'react';
import { TouchableHighlight, ViewStyle, TextStyle, View, Text} from 'react-native';

interface ButtonProps {
  onPressIn?: () => {},
  onPress?: () => {},
  title: string,
  wrapperStyle: ViewStyle;
  buttonTextStyle: TextStyle;

};

const Button = ({ title, onPressIn, onPress, wrapperStyle, buttonTextStyle }: ButtonProps) => {
  return <View>
    <TouchableHighlight onPressIn={onPressIn} onPress={onPress} style={wrapperStyle}>
      <Text style={buttonTextStyle}>
        {title}
      </Text>

    </TouchableHighlight>
  </View>
}

export default Button;