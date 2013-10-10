// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/download/download_started_animation.h"

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_view.h"
#include "grit/theme_resources.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/animation/linear_animation.h"
#include "ui/gfx/rect.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/widget/widget.h"

using content::WebContents;

// How long to spend moving downwards and fading out after waiting.
static const int kMoveTimeMs = 600;

// The animation framerate.
static const int kFrameRateHz = 60;

// What fraction of the frame height to move downward from the frame center.
// Note that setting this greater than 0.5 will mean moving past the bottom of
// the frame.
static const double kMoveFraction = 1.0 / 3.0;

namespace {

// DownloadStartAnimation creates an animation (which begins running
// immediately) that animates an image downward from the center of the frame
// provided on the constructor, while simultaneously fading it out.  To use,
// simply call "new DownloadStartAnimation"; the class cleans itself up when it
// finishes animating.
class DownloadStartedAnimationWin : public gfx::LinearAnimation,
                                    public content::WebContentsObserver,
                                    public views::ImageView {
 public:
  explicit DownloadStartedAnimationWin(WebContents* web_contents);

 private:
  // Move the animation to wherever it should currently be.
  void Reposition();

  // Shut down the animation cleanly.
  void Close();

  // Animation
  virtual void AnimateToState(double state) OVERRIDE;

  // content::WebContentsObserver
  virtual void WasHidden() OVERRIDE;
  virtual void WebContentsDestroyed(WebContents* web_contents) OVERRIDE;

  // We use a HWND for the popup so that it may float above any HWNDs in our UI.
  views::Widget* popup_;

  // The content area at the start of the animation. We store this so that the
  // download shelf's resizing of the content area doesn't cause the animation
  // to move around. This means that once started, the animation won't move
  // with the parent window, but it's so fast that this shouldn't cause too
  // much heartbreak.
  gfx::Rect web_contents_bounds_;

  DISALLOW_COPY_AND_ASSIGN(DownloadStartedAnimationWin);
};

DownloadStartedAnimationWin::DownloadStartedAnimationWin(
    WebContents* web_contents)
    : gfx::LinearAnimation(kMoveTimeMs, kFrameRateHz, NULL),
      content::WebContentsObserver(web_contents),
      popup_(NULL) {
  static gfx::ImageSkia* kDownloadImage = NULL;
  if (!kDownloadImage) {
    kDownloadImage = ui::ResourceBundle::GetSharedInstance().GetImageSkiaNamed(
        IDR_DOWNLOAD_ANIMATION_BEGIN);
  }

  // If we're too small to show the download image, then don't bother -
  // the shelf will be enough.
  web_contents->GetView()->GetContainerBounds(&web_contents_bounds_);
  if (web_contents_bounds_.height() < kDownloadImage->height())
    return;

  SetImage(kDownloadImage);

  popup_ = new views::Widget;

  views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
  params.opacity = views::Widget::InitParams::TRANSLUCENT_WINDOW;
  params.accept_events = false;
  params.parent = web_contents->GetView()->GetNativeView();
  popup_->Init(params);
  popup_->SetOpacity(0x00);
  popup_->SetContentsView(this);
  Reposition();
  popup_->Show();

  Start();
}

void DownloadStartedAnimationWin::Reposition() {
  if (!web_contents())
    return;

  // Align the image with the bottom left of the web contents (so that it
  // points to the newly created download).
  gfx::Size size = GetPreferredSize();
  int x = base::i18n::IsRTL() ?
      web_contents_bounds_.right() - size.width() : web_contents_bounds_.x();
  popup_->SetBounds(gfx::Rect(
      x,
      static_cast<int>(web_contents_bounds_.bottom() -
          size.height() - size.height() * (1 - GetCurrentValue())),
      size.width(),
      size.height()));
}

void DownloadStartedAnimationWin::Close() {
  if (!web_contents())
    return;

  Observe(NULL);
  popup_->Close();
}

void DownloadStartedAnimationWin::AnimateToState(double state) {
  if (state >= 1.0) {
    Close();
  } else {
    Reposition();

    // Start at zero, peak halfway and end at zero.
    double opacity = std::min(1.0 - pow(GetCurrentValue() - 0.5, 2) * 4.0,
                              static_cast<double>(1.0));

    popup_->SetOpacity(static_cast<unsigned char>(opacity * 255.0));
  }
}

void DownloadStartedAnimationWin::WasHidden() {
  Close();
}

void DownloadStartedAnimationWin::WebContentsDestroyed(
    WebContents* web_contents) {
  Close();
}

}  // namespace

// static
void DownloadStartedAnimation::Show(WebContents* web_contents) {
  // The animation will delete itself when it's finished or when the tab
  // contents is hidden or destroyed.
  new DownloadStartedAnimationWin(web_contents);
}
