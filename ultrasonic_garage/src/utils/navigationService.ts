import { NavigationActions, NavigationContainerComponent, NavigationParams } from 'react-navigation';

let _navigator: NavigationContainerComponent;

function setTopLevelNavigator(navigatorRef: NavigationContainerComponent) {
  _navigator = navigatorRef;
}

function navigate(routeName: string, params: NavigationParams) {
  _navigator.dispatch(
    NavigationActions.navigate({
      routeName,
      params,
    })
  );
}

export {
  setTopLevelNavigator,
  navigate
}
