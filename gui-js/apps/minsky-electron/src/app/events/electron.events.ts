/**
 * This module is responsible on handling all the inter process communications
 * between the frontend to the electron backend.
 */

import {
  AppLayoutPayload,
  CanvasItem,
  ChangeTabPayload,
  CppClass,
  environment,
  events,
  HandleDescriptionPayload,
  HandleDimensionPayload,
  PickSlicesPayload,
  MinskyProcessPayload,
  minsky,
  RenderNativeWindow,
} from '@minsky/shared';
//import * as debug from 'debug';
import { BrowserWindow, ipcMain } from 'electron';
import { BookmarkManager } from '../managers/BookmarkManager';
import { CommandsManager } from '../managers/CommandsManager';
import { ContextMenuManager } from '../managers/ContextMenuManager';
import { GodleyMenuManager } from '../managers/GodleyMenuManager';
import { KeyBindingsManager } from '../managers/KeyBindingsManager';
import { RecentFilesManager } from '../managers/RecentFilesManager';
import { StoreManager, MinskyPreferences } from '../managers/StoreManager';
import { WindowManager } from '../managers/WindowManager';

//const logUpdateEvent = debug('minsky:electron_update_event');

export default class ElectronEvents {
  static bootstrapElectronEvents(): Electron.IpcMain {
    return ipcMain;
  }
}

// Retrieve app version
ipcMain.handle(events.GET_APP_VERSION, () => {
  //logUpdateEvent(`Fetching application version... [v${environment.version}]`);

  return environment.version;
});

ipcMain.handle(events.BACKEND, (event, ...args: any[])=>{
  return CppClass.backend(...args);
});

ipcMain.on(events.SET_BACKGROUND_COLOR, async (event, { color }) => {
  if (color) {
    StoreManager.store.set('backgroundColor', color);
  }
  await CommandsManager.changeWindowBackgroundColor(
    StoreManager.store.get('backgroundColor')
  );
});

ipcMain.on(events.CREATE_MENU_POPUP, (event, data) => {
  WindowManager.createPopupWindowWithRouting(data);
});

// MINSKY_PROCESS_FOR_IPC_MAIN won't reply with the response
//ipcMain.on(
//  events.MINSKY_PROCESS_FOR_IPC_MAIN,
//  async (event, payload: MinskyProcessPayload) => {
//    await RestServiceManager.handleMinskyProcess(payload);
//  }
//);

//ipcMain.handle(
//  events.MINSKY_PROCESS,
//  async (event, payload: MinskyProcessPayload) => {
//    return await RestServiceManager.handleMinskyProcess(payload);
//  }
//);

ipcMain.on(
  events.APP_LAYOUT_CHANGED,
  async (event, payload: AppLayoutPayload) => {
    if (event.sender.id === WindowManager.getMainWindow().id) {
      WindowManager.onAppLayoutChanged(payload);
      WindowManager.renderFrame();
    }
  }
);

ipcMain.on(events.CHANGE_MAIN_TAB, async (event, payload: ChangeTabPayload) => {
  await WindowManager.setCurrentTab(new RenderNativeWindow(payload.newTab));
});

ipcMain.on(events.POPULATE_BOOKMARKS, async (event, bookmarks: string[]) => {
  await BookmarkManager.populateBookmarks(bookmarks);
});

ipcMain.on(
  events.INIT_MENU_FOR_GODLEY_VIEW,
  async (
    event,
    parameters: { window: BrowserWindow; itemInfo: CanvasItem }
  ) => {
    const menu = GodleyMenuManager.createMenusForGodleyView(
      parameters.window,
      parameters.itemInfo
    );
    WindowManager.storeWindowMenu(parameters.window, menu);
  }
);

ipcMain.handle(
  events.GODLEY_VIEW_MOUSEDOWN,async (event, payload: MinskyProcessPayload) => {
    GodleyMenuManager.mouseDown(payload.command,payload.mouseX,payload.mouseY);
  }
);

ipcMain.on(events.ADD_RECENT_FILE, (event, filePath: string) => {
  RecentFilesManager.addFileToRecentFiles(filePath);
});

ipcMain.handle(
  events.KEY_PRESS,
  async (event, payload: MinskyProcessPayload) => {
    // this is a asynchronous handler for events.KEY_PRESS
    return await KeyBindingsManager.handleOnKeyPress(payload);
  }
);

ipcMain.handle(
  events.SAVE_HANDLE_DESCRIPTION,
  async (event, payload: HandleDescriptionPayload) => {
    return await CommandsManager.saveHandleDescription(payload);
  }
);

ipcMain.handle(
  events.SAVE_HANDLE_DIMENSION,
  async (event, payload: HandleDimensionPayload) => {
    return await CommandsManager.saveHandleDimension(payload);
  }
);

ipcMain.handle(
  events.SAVE_PICK_SLICES,
  async (event, payload: PickSlicesPayload) => {
    return await CommandsManager.savePickSlices(payload);
  }
);

ipcMain.handle(
  events.CURRENT_TAB_POSITION,
  async (event)=>{
    return WindowManager.currentTab.position();
  }
);

ipcMain.handle(
  events.CURRENT_TAB_MOVE_TO,
  async (event, pos)=>{
    return WindowManager.currentTab.moveTo(pos[0],pos[1]);
  }
);

ipcMain.on(events.KEY_PRESS, async (event, payload: MinskyProcessPayload) => {
  // this is a synchronous handler for events.KEY_PRESS
  event.returnValue = await KeyBindingsManager.handleOnKeyPress(payload);
});

ipcMain.handle(events.GET_PREFERENCES, () => {
  return StoreManager.store.get('preferences');
});

ipcMain.handle(
  events.UPDATE_PREFERENCES,
  async (event, preferences: MinskyPreferences) => {
    const {
      enableMultipleEquityColumns,
      godleyTableShowValues,
      godleyTableOutputStyle,
      font,
    } = preferences;

    StoreManager.store.set('preferences', preferences);

    minsky.setGodleyDisplayValue(godleyTableShowValues,godleyTableOutputStyle);
    minsky.multipleEquities(enableMultipleEquityColumns);
    minsky.defaultFont(font);
    RecentFilesManager.updateNumberOfRecentFilesToDisplay();
    return;
  }
);

//ipcMain.on(events.AUTO_START_MINSKY_SERVICE, async () => {
//  await RestServiceManager.startMinskyService();
//});

ipcMain.handle(events.NEW_SYSTEM, async () => {
  await CommandsManager.createNewSystem();
  return;
});

ipcMain.handle(
  events.IMPORT_CSV,
  async (event, payload: MinskyProcessPayload) => {
    const { mouseX, mouseY } = payload;

    const itemInfo = await CommandsManager.getItemInfo(mouseX, mouseY);
    CommandsManager.importCSV(itemInfo, true);
    return;
  }
);

ipcMain.on(events.CONTEXT_MENU, async (event, { x, y, type, command }) => {
  await ContextMenuManager.initContextMenu(x, y, type, command);
});

ipcMain.on(
  events.DISPLAY_MOUSE_COORDINATES,
  async (event, { mouseX, mouseY }) => {
    WindowManager.showMouseCoordinateWindow({ mouseX, mouseY });
  }
);

ipcMain.on(events.DOUBLE_CLICK, async (event, payload) => {
  await CommandsManager.handleDoubleClick(payload);
});

ipcMain.on(events.LOG_SIMULATION, async (event, selectedItems: string[]) => {
  await CommandsManager.logSimulation(selectedItems);
});
