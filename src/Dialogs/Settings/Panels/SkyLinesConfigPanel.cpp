/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "SkyLinesConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Listener.hpp"
#include "Language/Language.hpp"
#include "Tracking/TrackingSettings.hpp"
#include "Tracking/SkyLines/Key.hpp"
#include "Net/State.hpp"
#include "Form/DataField/Base.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Screen/Layout.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Util/NumberParser.hpp"

enum ControlIndex {
  SL_ENABLED,
#ifdef HAVE_NET_STATE_ROAMING
  SL_ROAMING,
#endif
  SL_INTERVAL,
  SL_TRAFFIC_ENABLED,
  SL_NEAR_TRAFFIC_ENABLED,
  SL_KEY,
};

class SkyLinesConfigPanel final
  : public RowFormWidget, DataFieldListener {
public:
  SkyLinesConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  void SetSkyLinesEnabled(bool enabled);

  /* methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

void
SkyLinesConfigPanel::SetSkyLinesEnabled(bool enabled)
{
#ifdef HAVE_NET_STATE_ROAMING
  SetRowEnabled(SL_ROAMING, enabled);
#endif
  SetRowEnabled(SL_INTERVAL, enabled);
  SetRowEnabled(SL_TRAFFIC_ENABLED, enabled);
  SetRowEnabled(SL_NEAR_TRAFFIC_ENABLED,
                enabled && GetValueBoolean(SL_TRAFFIC_ENABLED));
  SetRowEnabled(SL_KEY, enabled);
}

void
SkyLinesConfigPanel::OnModified(DataField &df)
{
  if (IsDataField(SL_ENABLED, df)) {
    const DataFieldBoolean &dfb = (const DataFieldBoolean &)df;
    SetSkyLinesEnabled(dfb.GetAsBoolean());
    return;
  }

  if (IsDataField(SL_TRAFFIC_ENABLED, df)) {
    const DataFieldBoolean &dfb = (const DataFieldBoolean &)df;
    SetRowEnabled(SL_NEAR_TRAFFIC_ENABLED, dfb.GetAsBoolean());
    return;
  }
}

static constexpr StaticEnumChoice tracking_intervals[] = {
  { 1, _T("1 sec") },
  { 2, _T("2 sec") },
  { 3, _T("3 sec") },
  { 5, _T("5 sec") },
  { 10, _T("10 sec") },
  { 15, _T("15 sec") },
  { 20, _T("20 sec") },
  { 30, _T("30 sec") },
  { 45, _T("45 sec") },
  { 60, _T("1 min") },
  { 120, _T("2 min") },
  { 180, _T("3 min") },
  { 300, _T("5 min") },
  { 600, _T("10 min") },
  { 900, _T("15 min") },
  { 1200, _T("20 min") },
  { 0 },
};

void
SkyLinesConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const TrackingSettings &settings =
    CommonInterface::GetComputerSettings().tracking;

  RowFormWidget::Prepare(parent, rc);

  AddBoolean(_T("SkyLines"), nullptr, settings.skylines.enabled, this);
#ifdef HAVE_NET_STATE_ROAMING
  AddBoolean(_T("Roaming"), nullptr, settings.skylines.roaming, this);
#endif
  AddEnum(_("Tracking Interval"), nullptr, tracking_intervals,
          settings.skylines.interval);

  AddBoolean(_("Track friends"),
             _("Download the position of your friends live from the SkyLines server."),
             settings.skylines.traffic_enabled, this);

  AddBoolean(_("Show nearby traffic"),
             _("Download the position of your nearby traffic live from the SkyLines server."),
             settings.skylines.near_traffic_enabled, this);

  StaticString<64> buffer;
  if (settings.skylines.key != 0)
    buffer.UnsafeFormat(_T("%llX"), (unsigned long long)settings.skylines.key);
  else
    buffer.clear();
  AddText(_T("Key"), nullptr, buffer);

  SetSkyLinesEnabled(settings.skylines.enabled);
}

static bool
SaveKey(const RowFormWidget &form, unsigned idx, const char *profile_key,
        uint64_t &value_r)
{
  const TCHAR *const s = form.GetValueString(idx);
  uint64_t value = ParseUint64(s, nullptr, 16);
  if (value == value_r)
    return false;

  value_r = value;
  Profile::Set(profile_key, s);
  return true;
}

bool
SkyLinesConfigPanel::Save(bool &_changed)
{
  bool changed = false;

  TrackingSettings &settings =
    CommonInterface::SetComputerSettings().tracking;

  changed |= SaveValue(SL_ENABLED, ProfileKeys::SkyLinesTrackingEnabled,
                       settings.skylines.enabled);

#ifdef HAVE_NET_STATE_ROAMING
  changed |= SaveValue(SL_ROAMING, ProfileKeys::SkyLinesRoaming,
                       settings.skylines.roaming);
#endif

  changed |= SaveValue(SL_INTERVAL, ProfileKeys::SkyLinesTrackingInterval,
                       settings.skylines.interval);

  changed |= SaveValue(SL_TRAFFIC_ENABLED, ProfileKeys::SkyLinesTrafficEnabled,
                       settings.skylines.traffic_enabled);
  changed |= SaveValue(SL_NEAR_TRAFFIC_ENABLED,
                       ProfileKeys::SkyLinesNearTrafficEnabled,
                       settings.skylines.near_traffic_enabled);

  changed |= SaveKey(*this, SL_KEY, ProfileKeys::SkyLinesTrackingKey,
                     settings.skylines.key);

  _changed |= changed;

  return true;
}

Widget *
CreateSkyLinesConfigPanel()
{
  return new SkyLinesConfigPanel();
}
