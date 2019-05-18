import { AnyAction } from "redux";
import reduceReducers from "reduce-reducers";
import {
  GARAGE_DOOR_TOGGLE_SUCCESS,
  GARAGE_DOOR_TOGGLE_FAILURE,
  GARAGE_LIGHT_TOGGLE_SUCCESS,
  GARAGE_LIGHT_TOGGLE_FAILURE,
  STATUS_REFRESH_SUCCESS,
  STATUS_REFRESH_FAILURE
} from "./types";
import initialGarageStateInterface from "./interfaces";

export const initialGarageState: initialGarageStateInterface = {
  status: {
    door: 'Waiting for data...',
    light: 'Waiting for data...',
    temperature: 'Waiting for data...'
  },
  door_error: '',
  light_error: '',
  status_error: '',
  message: ''
};

const garageReducer = (state = initialGarageState, action: AnyAction) => {
  switch (action.type) {
    case GARAGE_DOOR_TOGGLE_SUCCESS:
      return { ...state, };
    case GARAGE_DOOR_TOGGLE_FAILURE:
      return { ...state, door_error: action.payload };
    case GARAGE_LIGHT_TOGGLE_SUCCESS:
      return { ...state, message: action.payload };
    case GARAGE_LIGHT_TOGGLE_FAILURE:
      return { ...state, light_error: action.payload };
    case STATUS_REFRESH_SUCCESS:
      return { ...state, status: action.payload };
    case STATUS_REFRESH_FAILURE:
      return { ...state, status_error: action.payload };
    default:
      return state;
  };
}
export default reduceReducers(garageReducer);
