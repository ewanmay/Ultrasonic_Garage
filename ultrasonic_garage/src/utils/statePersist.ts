import { AsyncStorage } from 'react-native'
// import { initialAuthStateInterface } from '../ducks/auth/interfaces';
import { initialPantryStateInterface} from '../ducks/pantry/interfaces'

interface state {
  pantry: initialPantryStateInterface
  mealPlan: {},
  recipes: {}
}

export async function loadState() {
  try {
    const serializedState = await AsyncStorage.getItem('state');
    if (!serializedState) {
      return undefined
    }
    return JSON.parse(serializedState)
  } catch (err) {
    return undefined
  }
}

export async function saveState(state: state) {
  try {
    await AsyncStorage.setItem('state', JSON.stringify(state));
  }
  catch (err) {
    console.log(err)
  }
}