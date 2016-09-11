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

#include "Livetrack24ConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Profile/Profile.hpp"
#include "Form/Edit.hpp"
#include "Form/DataField/Enum.hpp"
#include "Form/DataField/Boolean.hpp"
#include "Form/DataField/Listener.hpp"
#include "Language/Language.hpp"
#include "Tracking/TrackingSettings.hpp"
#include "Form/DataField/Base.hpp"
#include "Widget/RowFormWidget.hpp"
#include "Screen/Layout.hpp"
#include "Interface.hpp"
#include "UIGlobals.hpp"
#include "Util/NumberParser.hpp"

enum ControlIndex {
#if defined(HAVE_SKYLINES_TRACKING) && defined(HAVE_LIVETRACK24)
  SPACER,
#endif
#ifdef HAVE_LIVETRACK24
  LT24_ENABLED,
  LT24_INVERVAL,
  LT24_VEHICLE_TYPE,
  LT24_VEHICLE_NAME,
  LT24_SERVER,
  LT24_USERNAME,
  LT24_PASSWORD
#endif
};

class Livetrack24ConfigPanel final
  : public RowFormWidget, DataFieldListener {
public:
  Livetrack24ConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
#ifdef HAVE_LIVETRACK24
  void SetLiveTrack24Enabled(bool enabled);
#endif

  /* methods from Widget */
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc) override;
  virtual bool Save(bool &changed) override;

private:
  /* methods from DataFieldListener */
  virtual void OnModified(DataField &df) override;
};

#ifdef HAVE_LIVETRACK24

void
Livetrack24ConfigPanel::SetLiveTrack24Enabled(bool enabled)
{
  SetRowEnabled(LT24_INVERVAL, enabled);
  SetRowEnabled(LT24_VEHICLE_TYPE, enabled);
  SetRowEnabled(LT24_VEHICLE_NAME, enabled);
  SetRowEnabled(LT24_SERVER, enabled);
  SetRowEnabled(LT24_USERNAME, enabled);
  SetRowEnabled(LT24_PASSWORD, enabled);
}

#endif

void
Livetrack24ConfigPanel::OnModified(DataField &df)
{
#ifdef HAVE_LIVETRACK24
  if (IsDataField(LT24_ENABLED, df)) {
    const DataFieldBoolean &dfb = (const DataFieldBoolean &)df;
    SetLiveTrack24Enabled(dfb.GetAsBoolean());
  }
#endif
}

#ifdef HAVE_LIVETRACK24

static constexpr StaticEnumChoice server_list[] = {
  { 0, _T("www.livetrack24.com") },
  { 1, _T("test.livetrack24.com") },
  { 2, _T("livexc.dhv.de") },
  { 0 },
};

static constexpr StaticEnumChoice vehicle_type_list[] = {
  { (unsigned) LiveTrack24::Settings::VehicleType::GLIDER, N_("Glider") },
  { (unsigned) LiveTrack24::Settings::VehicleType::PARAGLIDER, N_("Paraglider") },
  { (unsigned) LiveTrack24::Settings::VehicleType::POWERED_AIRCRAFT, N_("Powered aircraft") },
  { (unsigned) LiveTrack24::Settings::VehicleType::HOT_AIR_BALLOON, N_("Hot-air balloon") },
  { (unsigned) LiveTrack24::Settings::VehicleType::HANGGLIDER_FLEX, N_("Hangglider (Flex/FAI1)") },
  { (unsigned) LiveTrack24::Settings::VehicleType::HANGGLIDER_RIGID, N_("Hangglider (Rigid/FAI5)") },
  { 0 },
};

#endif

void
Livetrack24ConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const TrackingSettings &settings =
    CommonInterface::GetComputerSettings().tracking;

  RowFormWidget::Prepare(parent, rc);

#if defined(HAVE_SKYLINES_TRACKING) && defined(HAVE_LIVETRACK24)
  AddSpacer();
#endif

#ifdef HAVE_LIVETRACK24
  AddBoolean(_T("LiveTrack24"),  _T(""), settings.livetrack24.enabled, this);

  AddTime(_("Tracking Interval"), _T(""), 5, 3600, 5, settings.livetrack24.interval);

  AddEnum(_("Vehicle Type"), _("Type of vehicle used."), vehicle_type_list,
          (unsigned) settings.livetrack24.vehicleType);
  AddText(_("Vehicle Name"), _T("Name of vehicle used."),
          settings.livetrack24.vehicle_name);

  WndProperty *edit = AddEnum(_("Server"), _T(""), server_list, 0);
  ((DataFieldEnum *)edit->GetDataField())->Set(settings.livetrack24.server);
  edit->RefreshDisplay();

  AddText(_("Username"), _T(""), settings.livetrack24.username);
  AddPassword(_("Password"), _T(""), settings.livetrack24.password);
#endif

#ifdef HAVE_LIVETRACK24
  SetLiveTrack24Enabled(settings.livetrack24.enabled);
#endif
}

bool
Livetrack24ConfigPanel::Save(bool &_changed)
{
  bool changed = false;

  TrackingSettings &settings =
    CommonInterface::SetComputerSettings().tracking;

#ifdef HAVE_LIVETRACK24
  changed |= SaveValue(LT24_INVERVAL, ProfileKeys::LiveTrack24TrackingInterval, settings.livetrack24.interval);

  changed |= SaveValueEnum(LT24_VEHICLE_TYPE, ProfileKeys::LiveTrack24TrackingVehicleType,
                           settings.livetrack24.vehicleType);

  changed |= SaveValue(LT24_VEHICLE_NAME, ProfileKeys::LiveTrack24TrackingVehicleName,
                       settings.livetrack24.vehicle_name);
#endif

#ifdef HAVE_LIVETRACK24
  changed |= SaveValue(LT24_ENABLED, ProfileKeys::LiveTrack24Enabled, settings.livetrack24.enabled);

  changed |= SaveValue(LT24_SERVER, ProfileKeys::LiveTrack24Server,
                       settings.livetrack24.server);

  changed |= SaveValue(LT24_USERNAME, ProfileKeys::LiveTrack24Username,
                       settings.livetrack24.username);

  changed |= SaveValue(LT24_PASSWORD, ProfileKeys::LiveTrack24Password,
                       settings.livetrack24.password);
#endif

  _changed |= changed;

  return true;
}

Widget *
CreateLivetrack24ConfigPanel()
{
  return new Livetrack24ConfigPanel();
}
