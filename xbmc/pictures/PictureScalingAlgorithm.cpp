/*
 *      Copyright (C) 2015 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <algorithm>

extern "C" {
#include "libswscale/swscale.h"
}

#include "PictureScalingAlgorithm.h"
#include "utils/StringUtils.h"

CPictureScalingAlgorithm::Algorithm CPictureScalingAlgorithm::Default = CPictureScalingAlgorithm::FastBilinear;

CPictureScalingAlgorithm::AlgorithmMap CPictureScalingAlgorithm::m_algorithms = {
  { FastBilinear,     { "fast_bilinear",    SWS_FAST_BILINEAR } },
  { Bilinear,         { "bilinear",         SWS_BILINEAR } },
  { Bicubic,          { "bicubic",          SWS_FAST_BILINEAR } },
  { Experimental,     { "experimental",     SWS_FAST_BILINEAR } },
  { NearestNeighbor,  { "nearest_neighbor", SWS_FAST_BILINEAR } },
  { AveragingArea,    { "averaging_area",   SWS_FAST_BILINEAR } },
  { Bicublin,         { "bicublin",         SWS_FAST_BILINEAR } },
  { Gaussian,         { "gaussian",         SWS_FAST_BILINEAR } },
  { Sinc,             { "sinc",             SWS_FAST_BILINEAR } },
  { Lanczos,          { "lanczos",          SWS_FAST_BILINEAR } },
  { BicubicSpline,    { "bicubic_spline",   SWS_FAST_BILINEAR } },
};

CPictureScalingAlgorithm::Algorithm CPictureScalingAlgorithm::FromString(const std::string& scalingAlgorithm)
{
  const auto& algorithm = std::find_if(m_algorithms.begin(), m_algorithms.end(),
    [&scalingAlgorithm](const std::pair<Algorithm, ScalingAlgorithm>& algo) { return StringUtils::EqualsNoCase(algo.second.name, scalingAlgorithm); });
  if (algorithm != m_algorithms.end())
    return algorithm->first;

  return None;
}

std::string CPictureScalingAlgorithm::ToString(Algorithm scalingAlgorithm)
{
  const auto& algorithm = m_algorithms.find(scalingAlgorithm);
  if (algorithm != m_algorithms.end())
    return algorithm->second.name;

  return "";
}

int CPictureScalingAlgorithm::ToSwscale(const std::string& scalingAlgorithm)
{
  return ToSwscale(FromString(scalingAlgorithm));
}

int CPictureScalingAlgorithm::ToSwscale(Algorithm scalingAlgorithm)
{
  const auto& algorithm = m_algorithms.find(scalingAlgorithm);
  if (algorithm != m_algorithms.end())
    return algorithm->second.swscale;

  return ToSwscale(Default);
}
