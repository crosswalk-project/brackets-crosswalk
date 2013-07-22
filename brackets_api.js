// Copyright (c) 2012 Adobe Systems Incorporated. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

var brackets = { app: {}, fs: {} };

brackets.app.NO_ERROR = 0;

brackets.fs.NO_ERROR = 0;
brackets.fs.ERR_UNKNOWN = 1;
brackets.fs.ERR_INVALID_PARAMS = 2;
brackets.fs.ERR_NOT_FOUND = 3;
brackets.fs.ERR_CANT_READ = 4;
brackets.fs.ERR_UNSUPPORTED_ENCODING = 5;
brackets.fs.ERR_CANT_WRITE = 6;
brackets.fs.ERR_OUT_OF_SPACE = 7;
brackets.fs.ERR_NOT_FILE = 8;
brackets.fs.ERR_NOT_DIRECTORY = 9;
brackets.fs.ERR_FILE_EXISTS = 10;

brackets.debugging_port = 0;

brackets._postMessage = (function() {
  brackets._callbacks = {};
  brackets._next_reply_id = 0;
  xwalk.setMessageListener('brackets', function(json) {
    var msg = JSON.parse(json);
    var reply_id = msg._reply_id;
    var callback = brackets._callbacks[reply_id];
    if (callback) {
      delete msg._reply_id;
      delete brackets._callbacks[reply_id];
      callback(msg);
    } else {
      console.log('Invalid reply_id received from Brackets extension: ' + reply_id);
    }
  });
  return function(msg, callback) {
    var reply_id = brackets._next_reply_id;
    brackets._next_reply_id += 1;
    brackets._callbacks[reply_id] = callback;
    msg._reply_id = reply_id.toString();
    xwalk.postMessage('brackets', JSON.stringify(msg));
  };
})();

// PROPERTIES
Object.defineProperty(brackets.app, "language", {
  writeable: false,
  get : function() { return (navigator.language).toLowerCase(); },
  enumerable : true,
  configurable : false
});

// SETUP MESSAGES
var msg = {
  'cmd': 'GetRemoteDebuggingPort'
};
brackets._postMessage(msg, function(r) {
  brackets.debugging_port = r.debugging_port;
});

// API
brackets.fs.readFile = function(path, encoding, callback) {
  var msg = {
    'cmd': 'ReadFile',
    'path': path,
    'encoding': encoding
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error, r.data);
  });
};

brackets.fs.stat = function(path, callback) {
  var msg = {
    'cmd': 'GetFileModificationTime',
    'path': path
  };
  brackets._postMessage(msg, function (r) {
    callback(r.error, {
      isFile: function () {
        return !r.is_dir;
      },
      isDirectory: function () {
        return r.is_dir;
      },
      mtime: new Date(r.modtime * 1000)
    });
  });
};

brackets.fs.readdir = function(path, callback) {
  var msg = {
    'cmd': 'ReadDir',
    'path': path
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error, r.files)
  });
};

brackets.fs.writeFile = function(path, data, encoding, callback) {
  var msg = {
    'cmd': 'WriteFile',
    'path': path,
    'data': data,
    'encoding': encoding
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.app.openLiveBrowser = function(url, enableRemoteDebugging, callback) {
  var msg = {
    'cmd': 'OpenLiveBrowser',
    'url': url
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.app.openURLInDefaultBrowser = function(callback, url) {
  var msg = {
    'cmd': 'OpenURLInDefaultBrowser',
    'url': url
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.fs.rename = function(oldPath, newPath, callback) {
  var msg = {
    'cmd': 'Rename',
    'oldPath': oldPath,
    'newPath': newPath
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.app.getApplicationSupportDirectory = function() {
  // FIXME(cmarcelo): Synchronous function. We need to store this
  // value when initializing the plugin.
  return '/tmp/brackets-support';
};

brackets.app.getNodeState = function(callback) {
  callback(true, 0);
};

brackets.app.quit = function() {
  window.close();
};

brackets.fs.makedir = function(path, mode, callback) {
  var msg = {
    'cmd': 'MakeDir',
    'path': path,
    'mode': mode
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.fs.unlink = function(path, callback) {
  var msg = {
    'cmd': 'DeleteFileOrDirectory',
    'path': path
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.fs.moveToTrash = function(path, callback) {
  var msg = {
    'cmd': 'MoveFileOrDirectoryToTrash',
    'path': path
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.fs.isNetworkDrive = function(path, callback) {
  var msg = {
    'cmd': 'IsNetworkDrive',
    'path': path
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error, r.is_network_drive);
  });
};

xwalk.menu = xwalk.menu || {};
xwalk.menu.onActivatedMenuItem = function(item) {
    brackets.shellAPI.executeCommand(item);
};

brackets.app.addMenu = function(title, id, position, relativeId, callback) {
  xwalk.menu.addMenu(title, id, position, relativeId);
  callback(brackets.app.NO_ERROR);
};

brackets.app.addMenuItem = function(parentId, title, id, key, displayStr, position, relativeId, callback) {
  xwalk.menu.addMenuItem(parentId, title, id, key, displayStr, position, relativeId);
  callback(brackets.app.NO_ERROR);
}

brackets.app.setMenuTitle = function(commandid, title, callback) {
  xwalk.menu.setMenuTitle(commandid, title);
  callback(brackets.app.NO_ERROR);
}

brackets.app.setMenuItemState = function(commandid, enabled, checked, callback) {
  xwalk.menu.setMenuItemState(commandid, enabled, checked);
  callback(brackets.app.NO_ERROR);
}

brackets.app.setMenuItemShortcut = function(commandid, shortcut, displayStr, callback) {
  xwalk.menu.setMenuItemShortcut(commandid, shortcut, displayStr);
  callback(brackets.app.NO_ERROR);
}

brackets.app.showOSFolder = function(path, callback) {
  var msg = {
    'cmd': 'ShowOSFolder',
    'path': path
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error);
  });
};

brackets.app.showExtensionsFolder = function(appURL, callback) {
  return brackets.app.showOSFolder(brackets.app.getApplicationSupportDirectory() + '/extensions', callback);
};

brackets.app.getPendingFilesToOpen = function(callback) {
  var msg = {
    'cmd': 'GetPendingFilesToOpen'
  };
  brackets._postMessage(msg, function(r) {
    callback(r.error, r.files);
  });
};

brackets.app.getRemoteDebuggingPort = function() {
  return brackets.debugging_port;
};

xwalk.experimental = xwalk.experimental || {};
xwalk.experimental.dialog = xwalk.experimental.dialog || {};
brackets.fs.showOpenDialog = function (allowMultipleSelection, chooseDirectory, title, initialPath, fileTypes, callback) {
  xwalk.experimental.dialog.showOpenDialog(allowMultipleSelection, chooseDirectory,
     title || 'Open', initialPath || '',
     fileTypes ? fileTypes.join(' ') : '', function(err, file) { callback(err, [file]); });
}

brackets.fs.showSaveDialog = function (title, initialPath, proposedNewFilename, callback) {
  xwalk.experimental.dialog.showSaveDialog(title || 'Save As', initialPath || '', proposedNewFilename || '', callback);
}
