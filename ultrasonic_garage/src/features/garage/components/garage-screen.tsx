import * as React from "react";
import { View, Text } from "react-native";
import { buttonStyles } from "../styles";
import { Icon } from "react-native-elements";
import Button from '../../common/button'
import initialGarageStateInterface from "../../../ducks/garage/interfaces";

interface screenComponent extends React.Component {
  navigationOptions?: Object;
}

interface garageProps {
  garage: initialGarageStateInterface,
  openDoor: () => {},
  closeDoor: () => {},
  turnOnLight: () => {},
  turnOffLight: () => {},
  retrieveStatus: () => {}
};

let interval = 0;
const renderScreen = (
  garage,
  openDoor,
  closeDoor,
  turnOnLight,
  turnOffLight,
  retrieveStatus) => {
    const {containerStyle, textStyle} = buttonStyles;
    clearInterval(interval)
    interval = setInterval(() => retrieveStatus(), 6000)

    const { door, light, temperature } = garage.status;
  return (
    <View>
      <Button
        onPressIn={() => openDoor()}
        onPress={() => closeDoor()}
        title={'Door'}
        wrapperStyle={containerStyle}
        buttonTextStyle={textStyle}
      />

      <Text style={textStyle}>
        {garage.door_error}
      </Text>
      <Button
        onPressIn={() => turnOnLight()}
        onPress={() => turnOffLight()}
        title={'Light'}
        wrapperStyle={containerStyle}
        buttonTextStyle={textStyle}
      />
      <Text style={textStyle}>
        {garage.light_error}
      </Text>
      <Text style={textStyle}>
        Door Status:
      </Text>
      <Text style={textStyle}>
        {door}
        {garage.door_error}
      </Text>
      <Text style={textStyle}>
        Light Status:
      </Text>
      <Text style={textStyle}>
        {light}
        {garage.door_error}
      </Text>
      
      <Text style={textStyle}>
        Temperature:
      </Text>
      <Text style={textStyle}>
        {temperature}
        {garage.door_error}
      </Text>
    </View>
  )

}

const GarageScreen = ({
  garage,
  openDoor,
  closeDoor,
  turnOnLight,
  turnOffLight,
  retrieveStatus

}: garageProps) => {
  return (
    <View style={buttonStyles.viewStyle}>
      {renderScreen(
        garage,
        openDoor,
        closeDoor,
        turnOnLight,
        turnOffLight,
        retrieveStatus)}
    </View>
  );
};


export default GarageScreen;
