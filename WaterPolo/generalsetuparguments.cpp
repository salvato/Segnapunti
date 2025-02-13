/*
 *
Copyright (C) 2023  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
#include <QStandardPaths>

#include "generalsetuparguments.h"


GeneralSetupArguments::GeneralSetupArguments()
    : maxTimeout(2)
    , maxPeriods(4)
    , iTimeDuration(8) //In minutes
    // The default Directories to look for the slides and spots
    , sSlideDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
    , sSpotDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
{
    sTeam[0] = "Locali";
    sTeam[1] = "Ospiti";
    sTeamLogoFilePath[0] = ":/../CommonFiles/Logo_SSD_UniMe.png";
    sTeamLogoFilePath[1] = ":/../CommonFiles/Logo_UniMe.png";
}
