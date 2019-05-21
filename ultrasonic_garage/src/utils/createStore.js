import { createStore, applyMiddleware } from "redux";
import reducers from "../ducks/reducers";
import ReduxThunk from "redux-thunk";
import {retrievePantry } from '../ducks/pantry/actions';
export default function configureStore(initialState) {
  let store = createStore(reducers, {}, applyMiddleware(ReduxThunk));

  };
  if (initialState) {
    store = createStore(reducers, initialState, applyMiddleware(ReduxThunk));
  }

  if (module.hot) {
    // Enable Webpack hot module replacement for reducers
    module.hot.accept("../ducks/reducers", () => {
      const nextRootReducer = require("../ducks/reducers");
      store.replaceReducer(nextRootReducer);
    });
  }

  return store;
}
