// Macros define at compilation time.
//
// Copyright (c) 2015 Idiap Research Institute, http://www.idiap.ch/
// Written by Rémi Lebret <remi@lebret.ch>
//
// This file is part of HPCA.
//
// HPCA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3 as
// published by the Free Software Foundation.
//
// HPCA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HPCA. If not, see <http://www.gnu.org/licenses/>.

#ifndef CONFIG_H
#define CONFIG_H

/*---- the configured options and settings for HPCA ---------------*/
/*---- the cmake configuration for NetSMF is adapted from HPCA ----*/

// should we use Intel MKL through Eigen?
#cmakedefine EIGEN_USE_MKL_ALL

#endif // CONFIG_H
