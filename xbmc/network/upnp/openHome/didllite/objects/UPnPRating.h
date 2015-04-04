#pragma once
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

#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

class CUPnPRating : public IDidlLiteElement
{
public:
  CUPnPRating();
  CUPnPRating(const std::string& rating, const std::string& type = "", const std::string& advice = "", const std::string& equivalentAge = "");
  CUPnPRating(const CUPnPRating& upnpRating);
  virtual ~CUPnPRating();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return new CUPnPRating(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPRating(*this); }

  const std::string& GetRating() const { return m_rating; }
  const std::string& GetRatingType() const { return m_ratingType; }
  const std::string& GetRatingAdvice() const { return m_ratingAdvice; }
  const std::string& GetRatingEquivalentAge() const { return m_ratingEquivalentAge; }

protected:
  // implementations of IDidlLiteElement
  virtual bool serialize(TiXmlElement* element) const;
  virtual bool deserialize(const TiXmlElement* element);

  void initializeProperties();

private:
  std::string m_rating;
  std::string m_ratingType;
  std::string m_ratingAdvice;
  std::string m_ratingEquivalentAge;
};
