#include "config.h"
#include "RenderTheme.h"

#include "CSSValueKeywords.h"
#include "Document.h"
#include "FloatConversion.h"
#include "FocusController.h"
#include "FontSelector.h"
#include "Frame.h"
#include "GraphicsContext.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "MediaControlElements.h"
#include "Page.h"
#include "PaintInfo.h"
#include "RenderStyle.h"
#include "RenderView.h"
#include "SelectionController.h"
#include "Settings.h"
#include "TextControlInnerElements.h"

#if ENABLE(METER_TAG)
#include "HTMLMeterElement.h"
#include "RenderMeter.h"
#endif

#if ENABLE(INPUT_SPEECH)
#include "RenderInputSpeech.h"
#endif
#if USE(WRATH)

#include "PaintInfoOfWRATH.h"
namespace 
{
  class RenderTheme_WRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderTheme_WRATHWidgets>
  {
  public:

    void
    make_all_handles_non_visible(void)
    {
      for(int i=0, endi=m_handles.size(); i<endi; ++i)
        {
          m_handles[i].visible(false);
        }
    }

    /*
      ControlPart is an enumeration defined in platform/ThemeTypes.h, 
      NumberControlPartEnums is the "last" one
    */
    vecN<WebCore::PaintedWidgetsOfWRATHHandle, WebCore::NumberControlPartEnums> m_handles; 
  };
}

namespace WebCore {

bool RenderTheme::readyWRATHWidgets(RenderObject *o, PaintedWidgetsOfWRATHHandle &handle,
                                    const PaintInfoOfWRATH &paintInfo, const IntRect &r)
{
  RenderTheme_WRATHWidgets *d;
  d=RenderTheme_WRATHWidgets::object(o, handle);
  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->make_all_handles_non_visible();
  ControlPart part = o->style()->appearance();
  bool return_value;

  // Call the appropriate paint method based off the appearance value.
  switch (part) {
#if !USE(NEW_THEME)
  case CheckboxPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetCheckbox(d->m_handles[part], o, paintInfo, r);
    break;

  case RadioPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetRadio(d->m_handles[part], o, paintInfo, r);
    break;

  case PushButtonPart:
  case SquareButtonPart:
  case ListButtonPart:
  case DefaultButtonPart:
  case ButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetButton(d->m_handles[part], o, paintInfo, r);
    break;

  case InnerSpinButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetInnerSpinButton(d->m_handles[part], o, paintInfo, r);
    break;

  case OuterSpinButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetOuterSpinButton(d->m_handles[part], o, paintInfo, r);
    break;

#endif
  case MenulistPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMenuList(d->m_handles[part], o, paintInfo, r);
    break;

#if ENABLE(METER_TAG)
  case MeterPart:
  case RelevancyLevelIndicatorPart:
  case ContinuousCapacityLevelIndicatorPart:
  case DiscreteCapacityLevelIndicatorPart:
  case RatingLevelIndicatorPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMeter(d->m_handles[part], o, paintInfo, r);
    break;
#endif

#if ENABLE(PROGRESS_TAG)
  case ProgressBarPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetProgressBar(d->m_handles[part], o, paintInfo, r);
    break;
#endif

  case SliderHorizontalPart:
  case SliderVerticalPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetSliderTrack(d->m_handles[part], o, paintInfo, r);
    break;

  case SliderThumbHorizontalPart:
  case SliderThumbVerticalPart:
    if (o->parent()->isSlider()) 
      {
        d->m_handles[part].visible(true);
        return_value=readyWRATHWidgetSliderThumb(d->m_handles[part], o, paintInfo, r);
      }
    else
      return_value=true;
    // We don't support drawing a slider thumb without a parent slider
    break;
  case MediaFullscreenButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaFullscreenButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaPlayButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaPlayButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaMuteButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaMuteButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaSeekBackButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaSeekBackButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaSeekForwardButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaSeekForwardButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaRewindButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaRewindButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaReturnToRealtimeButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaReturnToRealtimeButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaToggleClosedCaptionsButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaToggleClosedCaptionsButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaSliderPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaSliderTrack(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaSliderThumbPart:
    if (o->parent()->isSlider())
      {
        d->m_handles[part].visible(true);
        return_value=readyWRATHWidgetMediaSliderThumb(d->m_handles[part], o, paintInfo, r);
      }
    else
      return_value=true;
    break;

  case MediaVolumeSliderMuteButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaMuteButton(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaVolumeSliderContainerPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaVolumeSliderContainer(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaVolumeSliderPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaVolumeSliderTrack(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaVolumeSliderThumbPart:
    if (o->parent()->isSlider())
      {
        d->m_handles[part].visible(true);
        return_value=readyWRATHWidgetMediaVolumeSliderThumb(d->m_handles[part], o, paintInfo, r);
      }
    else
      return_value=true;
    break;

  case MediaTimeRemainingPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaTimeRemaining(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaCurrentTimePart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaCurrentTime(d->m_handles[part], o, paintInfo, r);
    break;

  case MediaControlsBackgroundPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetMediaControlsBackground(d->m_handles[part], o, paintInfo, r);
    break;

  case MenulistButtonPart:
  case TextFieldPart:
  case TextAreaPart:
  case ListboxPart:
    return_value=true;
    break;

  case SearchFieldPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetSearchField(d->m_handles[part], o, paintInfo, r);
    break;

  case SearchFieldCancelButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetSearchFieldCancelButton(d->m_handles[part], o, paintInfo, r);
    break;

  case SearchFieldDecorationPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetSearchFieldDecoration(d->m_handles[part], o, paintInfo, r);
    break;

  case SearchFieldResultsDecorationPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetSearchFieldResultsDecoration(d->m_handles[part], o, paintInfo, r);
    break;

  case SearchFieldResultsButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetSearchFieldResultsButton(d->m_handles[part], o, paintInfo, r);
    break;

#if ENABLE(INPUT_SPEECH)
  case InputSpeechButtonPart:
    d->m_handles[part].visible(true);
    return_value=readyWRATHWidgetInputFieldSpeechButton(d->m_handles[part], o, paintInfo, r);
    break;
#endif
  default:
    return_value=true; // We don't support the appearance, so let the normal background/border paint.
    break;
  } //of switch
  
  
  handle.visible(!return_value);
  return return_value;
}


bool RenderTheme::readyWRATHWidgetBorderOnly(RenderObject *o, PaintedWidgetsOfWRATHHandle &handle,
                                             const PaintInfoOfWRATH &paintInfo, const IntRect &r)
{
  RenderTheme_WRATHWidgets *d;
  d=RenderTheme_WRATHWidgets::object(o, handle);
  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->make_all_handles_non_visible();
  ControlPart part = o->style()->appearance();
  bool return_value;

  // Call the appropriate paint method based off the appearance value.
  switch (o->style()->appearance()) 
    {
    case TextFieldPart:
      d->m_handles[part].visible(true);
      return_value=readyWRATHWidgetTextField(d->m_handles[part], o, paintInfo, r);
      break;
      
    case ListboxPart:
    case TextAreaPart:
      d->m_handles[part].visible(true);
      return_value=readyWRATHWidgetTextArea(d->m_handles[part], o, paintInfo, r);
      break;
      
    case MenulistButtonPart:
    case SearchFieldPart:
      return_value=true;
      break;
      
    case CheckboxPart:
    case RadioPart:
    case PushButtonPart:
    case SquareButtonPart:
    case ListButtonPart:
    case DefaultButtonPart:
    case ButtonPart:
    case MenulistPart:
#if ENABLE(METER_TAG)
    case MeterPart:
    case RelevancyLevelIndicatorPart:
    case ContinuousCapacityLevelIndicatorPart:
    case DiscreteCapacityLevelIndicatorPart:
    case RatingLevelIndicatorPart:
#endif
#if ENABLE(PROGRESS_TAG)
    case ProgressBarPart:
#endif
    case SliderHorizontalPart:
    case SliderVerticalPart:
    case SliderThumbHorizontalPart:
    case SliderThumbVerticalPart:
    case SearchFieldCancelButtonPart:
    case SearchFieldDecorationPart:
    case SearchFieldResultsDecorationPart:
    case SearchFieldResultsButtonPart:
#if ENABLE(INPUT_SPEECH)
    case InputSpeechButtonPart:
#endif
    default:
      return_value=false;
      break;
    } //of swith
  
  handle.visible(!return_value);
  return return_value;
}

bool RenderTheme::readyWRATHWidgetDecorations(RenderObject *o, PaintedWidgetsOfWRATHHandle &handle,
                                              const PaintInfoOfWRATH &paintInfo, const IntRect &r)
{
  RenderTheme_WRATHWidgets *d;
  d=RenderTheme_WRATHWidgets::object(o, handle);
  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->make_all_handles_non_visible();
  ControlPart part = o->style()->appearance();
  bool return_value;
  
  switch (o->style()->appearance()) 
    {
    case MenulistButtonPart:
      d->m_handles[part].visible(true);
      return_value=readyWRATHWidgetMenuListButton(d->m_handles[part], o, paintInfo, r);
      break;
    case TextFieldPart:
    case TextAreaPart:
    case ListboxPart:
    case CheckboxPart:
    case RadioPart:
    case PushButtonPart:
    case SquareButtonPart:
    case ListButtonPart:
    case DefaultButtonPart:
    case ButtonPart:
    case MenulistPart:
#if ENABLE(METER_TAG)
    case MeterPart:
    case RelevancyLevelIndicatorPart:
    case ContinuousCapacityLevelIndicatorPart:
    case DiscreteCapacityLevelIndicatorPart:
    case RatingLevelIndicatorPart:
#endif
#if ENABLE(PROGRESS_TAG)
    case ProgressBarPart:
#endif
    case SliderHorizontalPart:
    case SliderVerticalPart:
    case SliderThumbHorizontalPart:
    case SliderThumbVerticalPart:
    case SearchFieldPart:
    case SearchFieldCancelButtonPart:
    case SearchFieldDecorationPart:
    case SearchFieldResultsDecorationPart:
    case SearchFieldResultsButtonPart:
#if ENABLE(INPUT_SPEECH)
    case InputSpeechButtonPart:
#endif
    default:
      return_value=false;
      break;
    } //of switch
  
  handle.visible(!return_value);
  return return_value;
}

}


#endif
