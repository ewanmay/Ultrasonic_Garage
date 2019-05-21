import { combineReducers } from 'redux';
import garageReducer from './garage';
export default combineReducers({
    garage: garageReducer
});