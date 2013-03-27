// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_MESSAGE_CENTER_COCOA_NOTIFICATION_CONTROLLER_H_
#define UI_MESSAGE_CENTER_COCOA_NOTIFICATION_CONTROLLER_H_

#import <Cocoa/Cocoa.h>

#import "base/memory/scoped_nsobject.h"
#include "ui/message_center/notification.h"

@class HoverImageButton;

// The base view controller class for notifications. A notification at minimum
// has an image, title, body, and close button. This controller can be used as
// the content for both a popup bubble and a view in the notification tray.
@interface MCNotificationController : NSViewController {
 @protected
  // The message object.
  const message_center::Notification* notification_;

  // The button that invokes |-close:|, in the upper-right corner.
  scoped_nsobject<HoverImageButton> closeButton_;

  // The large icon associated with the notification, on the left side.
  scoped_nsobject<NSImageView> icon_;

  // The title of the message.
  scoped_nsobject<NSTextField> title_;

  // Body text of the message.
  scoped_nsobject<NSTextField> message_;
}

// Creates a new controller for a given notification.
- (id)initWithNotification:(const message_center::Notification*)notification;

// Action for clicking on the notification's |closeButton_|.
- (void)close:(id)sender;

@end

#endif  // UI_MESSAGE_CENTER_COCOA_NOTIFICATION_CONTROLLER_H_
