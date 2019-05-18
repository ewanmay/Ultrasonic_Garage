import {
  GARAGE_DOOR_TOGGLE_SUCCESS,
  GARAGE_DOOR_TOGGLE_FAILURE,
  GARAGE_LIGHT_TOGGLE_SUCCESS,
  GARAGE_LIGHT_TOGGLE_FAILURE,
  STATUS_REFRESH_SUCCESS,
  STATUS_REFRESH_FAILURE
} from "./types";

import { navigate } from "../../utils/navigationService";
import axios from 'axios'
import { Dispatch } from "redux";

export const openDoor = () => async (dispatch: Dispatch) => {
  try {
    const instance = axios.create({
      baseURL: 'http://blynk-cloud.com/dcaf06230d2d4a83b47d625bb2e904ce',
      timeout: 4000
    })
    const res = await instance.get('/update/V11?value=1');
    console.log(res);
  }
  catch(e) {
    dispatch({
      type: GARAGE_DOOR_TOGGLE_SUCCESS,
      payload: 'Failed to toggle'
    })
  }
}

export const closeDoor = () => async (dispatch: Dispatch) => {
  try {
    const instance = axios.create({
      baseURL: 'http://blynk-cloud.com/dcaf06230d2d4a83b47d625bb2e904ce',
      timeout: 1000
    })
    const res = await instance.get('/update/V11?value=0');
    console.log(res);
    
    dispatch({
      type: GARAGE_DOOR_TOGGLE_SUCCESS,
      payload: 'Successful toggle'
    });
    return;
  }
  catch(e) {
    dispatch({
      type: GARAGE_DOOR_TOGGLE_FAILURE,
      payload: 'Failed to toggle'
    })
  }
}

export const turnOnLight = () => async (dispatch: Dispatch) => {
  try {
    const instance = axios.create({
      baseURL: 'http://blynk-cloud.com/dcaf06230d2d4a83b47d625bb2e904ce',
      timeout: 1000
    })
    const res = await instance.get('/update/V5?value=1');
    console.log(res);
    
    dispatch({
      type: GARAGE_LIGHT_TOGGLE_SUCCESS,
      payload: 'Successful toggle'
    });
    return;
  }
  catch(e) {
    dispatch({
      type: GARAGE_LIGHT_TOGGLE_FAILURE,
      payload: 'Failed to toggle'
    })
  }
}

export const turnOffLight = () => async (dispatch: Dispatch) => {
  try {
    const instance = axios.create({
      baseURL: 'http://blynk-cloud.com/dcaf06230d2d4a83b47d625bb2e904ce',
      timeout: 1000
    })
    const res = await instance.get('/update/V5?value=0');
    console.log(res);
    
    dispatch({
      type: GARAGE_DOOR_TOGGLE_SUCCESS,
      payload: 'Successful toggle'
    })
    return;
  }
  catch(e) {
    dispatch({
      type: GARAGE_DOOR_TOGGLE_SUCCESS,
      payload: 'Failed to toggle'
    })
  }
}

export const retrieveStatus = () => async (dispatch: Dispatch) => {
  try {
    const instance = axios.create({
      baseURL: 'http://blynk-cloud.com/dcaf06230d2d4a83b47d625bb2e904ce',
      timeout: 1000
    })
    let res = await instance.get('/get/V4');
    const door = res.data[0];
    res = await instance.get('/get/V7');
    const light = res.data[0];
    res = await instance.get('/get/V6');
    const temperature = res.data[0];
    dispatch({
      type: STATUS_REFRESH_SUCCESS,
      payload: {
        door,
        light,
        temperature
      }
    })
    console.log(res);
    return;
  } catch (e) {
    dispatch({
      type: STATUS_REFRESH_FAILURE,
      payload: e
    })
    return;
  }
}

export default {
  openDoor, closeDoor, turnOffLight, turnOnLight, retrieveStatus
}