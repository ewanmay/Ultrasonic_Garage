import GarageScreen from "../components/garage-screen";
import { connect } from "react-redux";
import { garageActions } from '../../../ducks/garage/index'
import stateInterface from '../../../ducks/interface'
const mapStateToProps = ({ garage }: stateInterface) => {
  return {
    garage
  }
};

const mapDispatchToProps = (dispatch: any) => {
  const { openDoor,
    closeDoor,
    turnOnLight,
    turnOffLight,
    retrieveStatus } = garageActions;
  return {
    openDoor: () => {
      dispatch(openDoor());
    },
    closeDoor: () => {
      dispatch(closeDoor());
    },
    turnOnLight: () => {
      dispatch(turnOnLight());
    },
    turnOffLight: () => {
      dispatch(turnOffLight());
    },
    retrieveStatus: () => {
      dispatch(retrieveStatus());
    }
  }
};

export default connect(
  mapStateToProps,
  mapDispatchToProps
)(GarageScreen)
