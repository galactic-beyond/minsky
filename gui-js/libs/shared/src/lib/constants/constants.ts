export const rendererAppPort = 4200;
export const rendererAppURL = `http://localhost:${rendererAppPort}`;
export const rendererAppName = 'minsky-web';
export const electronAppName = 'minsky-electron';
export const backgroundColor = '#c1c1c1';
export const updateServerUrl = 'https://deployment-server-url.com'; // TODO: insert your update server url here

export const defaultBackgroundColor = '#ffffff';

export const ZOOM_IN_FACTOR = 1.1;
export const ZOOM_OUT_FACTOR = 0.91;
export const RESET_ZOOM_FACTOR = 1;

export const OPEN_DEV_TOOLS_IN_DEV_BUILD = false;

export const events = {
  BACKEND: 'backend',
  SET_BACKGROUND_COLOR: 'set-background-color',
  CREATE_MENU_POPUP: 'create-menu-popup',
  APP_LAYOUT_CHANGED: 'app-layout-changed',
  UPDATE_BOOKMARK_LIST: 'update-bookmark-list',
  ADD_RECENT_FILE: 'add-recent-file',
  GET_APP_VERSION: 'get-app-version',
  KEY_PRESS: 'key-press',
  NEW_SYSTEM: 'new-system',
  RECORDING_STATUS_CHANGED: 'recording-status-changed',
  GET_PREFERENCES: 'get-preferences',
  UPDATE_PREFERENCES: 'update-preferences',
  CONTEXT_MENU: 'context-menu',
  REPLAY_RECORDING: 'replay-recording',
  DISPLAY_MOUSE_COORDINATES: 'display-mouse-coordinates',
  DOUBLE_CLICK: 'double-click',
  LOG_SIMULATION: 'log-simulation',
  CHANGE_MAIN_TAB: 'change-main-tab',
  IMPORT_CSV: 'import-csv',
  INIT_MENU_FOR_GODLEY_VIEW: 'init-menu-for-godley-view',
  GODLEY_VIEW_MOUSEDOWN: 'godley-view-mousedown',
  CURRENT_TAB_POSITION: 'current-tab-position',
  CURRENT_TAB_MOVE_TO: 'current-tab-move-to',
  RECORD: 'record',
  RECORDING_REPLAY: 'recording-replay',
  SAVE_HANDLE_DESCRIPTION: 'save-handle-description',
  SAVE_HANDLE_DIMENSION: 'save-handle-dimension',
  SAVE_PICK_SLICES: 'save-pick-slices',
};

// add non exposed commands here to get intellisense on the terminal popup
export const unExposedTerminalCommands = [
  '/minsky/model/cBounds',
  '/minsky/model/zoomFactor',
  '/minsky/model/relZoom',
  '/minsky/model/setZoom',
  '/minsky/canvas/itemFocus/initValue',
  '/minsky/canvas/itemFocus/tooltip',
  '/minsky/canvas/itemFocus/detailedText',
  '/minsky/canvas/itemFocus/sliderMax',
  '/minsky/canvas/itemFocus/sliderMin',
  '/minsky/canvas/itemFocus/sliderStep',
  '/minsky/canvas/itemFocus/sliderStepRel',
  '/minsky/canvas/itemFocus/rotation',
  '/minsky/canvas/itemFocus/setUnits',
];

export const dateTimeFormats = [
  { label: '1999-Q4', value: '%Y-Q%Q' },
  { label: '1999', value: '%Y' },
  { label: '12/31/99', value: '%m/%d/%y' },
  { label: '12/31/1999', value: '%m/%d/%Y' },
  { label: '31/12/99', value: '%d/%m/%y' },
  { label: '31/12/1999', value: '%d/%m/%Y' },
  { label: '1999-12-31T13:37:46', value: '%Y-%m-%dT%H:%M:%S' },
  { label: '1999-12-31 13:37:46', value: '%Y-%m-%dT%H:%M:%S' },
  { label: '12/31/1999 01:37 PM', value: '%m/%d/%Y %I:%M %p' },
  { label: '12/31/99 01:37 PM', value: '%m/%d/%y %I:%M %p' },
  { label: '12/31/1999 13:37 PM', value: '%m/%d/%Y %H:%M %p' },
  { label: '12/31/99 13:37 PM', value: '%m/%d/%y %H:%M %p' },
  { label: 'Friday, December 31, 1999', value: '%A, %B %d, %Y' },
  { label: 'Dec 31, 99', value: '%b %d, %y' },
  { label: 'Dec 31, 1999', value: '%b %d, %Y' },
  { label: '31. Dec. 1999', value: '%d. %b. %Y' },
  { label: 'December 31, 1999', value: '%B %d, %Y' },
  { label: '31. December 1999', value: '%d. %B %Y' },
  { label: 'Fri, Dec 31, 99', value: '%a, %b %d, %y' },
  { label: 'Fri 31/Dec 99', value: '%a %d/%b %y' },
  { label: 'Fri, Dec 31, 1999', value: '%a, %b %d, %Y' },
  { label: 'Friday, December 31, 1999', value: '%A, %B %d, %Y' },
  { label: '12-31', value: '%m-%d' },
  { label: '99-12-31', value: '%y-%m-%d' },
  { label: '1999-12-31', value: '%Y-%m-%d' },
  { label: '12/99', value: '%m/%y' },
  { label: 'Dec 31', value: '%b %d' },
  { label: 'December', value: '%B' },
  { label: '4th quarter 99', value: '%Qth quarter %y' },
];

export const importCSVvariableName = 'dataImport';
