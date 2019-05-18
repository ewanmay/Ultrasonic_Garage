import {StyleSheet} from 'react-native';
export const colors = {
  black: "#23292E",
  gray: "#d5d5d5",
  lightGreen: "#55ad7a",
  paleBlue: "#1e5d88",
  darkerBlue: "#334d5c"
};

export const buttonStyles = StyleSheet.create({
  containerStyle: {
    backgroundColor: colors.lightGreen,
    padding: 25,
    marginTop: 45,
    margin: 25,
    position: 'relative',
    flexDirection: 'row',
    justifyContent: 'center',
    borderRadius: 10
  },  
  textStyle: {
    color: "black",
    padding: 6,
    fontWeight: "bold",
    letterSpacing: 1,
    textTransform: 'uppercase',
    fontSize: 16,
    // fontFamily: "questrial",
  },
  
  viewStyle: {
    alignSelf: "stretch",
    backgroundColor: 'white',
    borderBottomColor: colors.paleBlue,
    borderBottomWidth: 2
  },  
});
