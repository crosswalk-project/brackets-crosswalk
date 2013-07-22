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

exports.app = {
  NO_ERROR: 0,
};

exports.fs = {
  NO_ERROR: 0,
  ERR_UNKNOWN: 1,
  ERR_INVALID_PARAMS: 2,
  ERR_NOT_FOUND: 3,
  ERR_CANT_READ: 4,
  ERR_UNSUPPORTED_ENCODING: 5,
  ERR_CANT_WRITE: 6,
  ERR_OUT_OF_SPACE: 7,
  ERR_NOT_FILE: 8,
  ERR_NOT_DIRECTORY: 9,
  ERR_FILE_EXISTS: 10,
};

var postMessage = (function() {
  var callbacks = {};
  var next_reply_id = 0;
  extension.setMessageListener(function(json) {
    var msg = JSON.parse(json);
    var reply_id = msg._reply_id;
    var callback = callbacks[reply_id];
    if (callback) {
      delete msg._reply_id;
      delete callbacks[reply_id];
      callback(msg);
    } else {
      console.log('Invalid reply_id received from Brackets extension: ' + reply_id);
    }
  });
  return function(msg, callback) {
    var reply_id = next_reply_id;
    next_reply_id += 1;
    callbacks[reply_id] = callback;
    msg._reply_id = reply_id.toString();
    extension.postMessage(JSON.stringify(msg));
  }
})();

// PROPERTIES
Object.defineProperty(exports.app, "language", {
  writeable: false,
  get : function() { return (navigator.language).toLowerCase(); },
  enumerable : true,
  configurable : false
});

// SETUP MESSAGES
var debugging_port = 0;
var msg = {
  'cmd': 'GetRemoteDebuggingPort'
};
postMessage(msg, function(r) {
  debugging_port = r.debugging_port;
});

// API
exports.fs.readFile = function(path, encoding, callback) {
  var msg = {
    'cmd': 'ReadFile',
    'path': path,
    'encoding': encoding
  };
  postMessage(msg, function(r) {
    callback(r.error, r.data);
  });
};

exports.fs.stat = function(path, callback) {
  var msg = {
    'cmd': 'GetFileModificationTime',
    'path': path
  };
  postMessage(msg, function (r) {
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

exports.fs.readdir = function(path, callback) {
  var msg = {
    'cmd': 'ReadDir',
    'path': path
  };
  postMessage(msg, function(r) {
    callback(r.error, r.files)
  });
};

exports.fs.writeFile = function(path, data, encoding, callback) {
  var msg = {
    'cmd': 'WriteFile',
    'path': path,
    'data': data,
    'encoding': encoding
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.app.openLiveBrowser = function(url, enableRemoteDebugging, callback) {
  var msg = {
    'cmd': 'OpenLiveBrowser',
    'url': url
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.app.openURLInDefaultBrowser = function(callback, url) {
  var msg = {
    'cmd': 'OpenURLInDefaultBrowser',
    'url': url
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.fs.rename = function(oldPath, newPath, callback) {
  var msg = {
    'cmd': 'Rename',
    'oldPath': oldPath,
    'newPath': newPath
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.app.getApplicationSupportDirectory = function() {
  // FIXME(cmarcelo): Synchronous function. We need to store this
  // value when initializing the plugin.
  return '/tmp/brackets-support';
};

exports.app.getNodeState = function(callback) {
  callback(true, 0);
};

exports.app.quit = function() {
  window.close();
};

exports.fs.makedir = function(path, mode, callback) {
  var msg = {
    'cmd': 'MakeDir',
    'path': path,
    'mode': mode
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.fs.unlink = function(path, callback) {
  var msg = {
    'cmd': 'DeleteFileOrDirectory',
    'path': path
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.fs.moveToTrash = function(path, callback) {
  var msg = {
    'cmd': 'MoveFileOrDirectoryToTrash',
    'path': path
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.fs.isNetworkDrive = function(path, callback) {
  var msg = {
    'cmd': 'IsNetworkDrive',
    'path': path
  };
  postMessage(msg, function(r) {
    callback(r.error, r.is_network_drive);
  });
};

xwalk.menu = xwalk.menu || {};
xwalk.menu.onActivatedMenuItem = function(item) {
    brackets.shellAPI.executeCommand(item);
};

exports.app.addMenu = function(title, id, position, relativeId, callback) {
  xwalk.menu.addMenu(title, id, position, relativeId);
  callback(exports.app.NO_ERROR);
};

exports.app.addMenuItem = function(parentId, title, id, key, displayStr, position, relativeId, callback) {
  xwalk.menu.addMenuItem(parentId, title, id, key, displayStr, position, relativeId);
  callback(exports.app.NO_ERROR);
}

exports.app.setMenuTitle = function(commandid, title, callback) {
  xwalk.menu.setMenuTitle(commandid, title);
  callback(exports.app.NO_ERROR);
}

exports.app.setMenuItemState = function(commandid, enabled, checked, callback) {
  xwalk.menu.setMenuItemState(commandid, enabled, checked);
  callback(exports.app.NO_ERROR);
}

exports.app.setMenuItemShortcut = function(commandid, shortcut, displayStr, callback) {
  xwalk.menu.setMenuItemShortcut(commandid, shortcut, displayStr);
  callback(exports.app.NO_ERROR);
}

exports.app.showOSFolder = function(path, callback) {
  var msg = {
    'cmd': 'ShowOSFolder',
    'path': path
  };
  postMessage(msg, function(r) {
    callback(r.error);
  });
};

exports.app.showExtensionsFolder = function(appURL, callback) {
  return exports.app.showOSFolder(exports.app.getApplicationSupportDirectory() + '/extensions', callback);
};

exports.app.getPendingFilesToOpen = function(callback) {
  var msg = {
    'cmd': 'GetPendingFilesToOpen'
  };
  postMessage(msg, function(r) {
    callback(r.error, r.files);
  });
};

exports.app.getRemoteDebuggingPort = function() {
  return debugging_port;
};

xwalk.experimental = xwalk.experimental || {};
xwalk.experimental.dialog = xwalk.experimental.dialog || {};
exports.fs.showOpenDialog = function (allowMultipleSelection, chooseDirectory, title, initialPath, fileTypes, callback) {
  xwalk.experimental.dialog.showOpenDialog(allowMultipleSelection, chooseDirectory,
     title || 'Open', initialPath || '',
     fileTypes ? fileTypes.join(' ') : '', callback);
}

exports.fs.showSaveDialog = function (title, initialPath, proposedNewFilename, callback) {
  xwalk.experimental.dialog.showSaveDialog(title || 'Save As', initialPath || '', proposedNewFilename || '', callback);
}
