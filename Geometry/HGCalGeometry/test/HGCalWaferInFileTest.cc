// -*- C++ -*-
//
// Package:    HGCalWaferInFileTest
// Class:      HGCalWaferInFileTest
//
/**\class HGCalWaferInFileTest HGCalWaferInFileTest.cc
 test/HGCalWaferInFileTest.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Sunanda Banerjee
//         Created:  Mon 2020/06/24
//
//

// system include files
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "Geometry/HGCalCommonData/interface/HGCalDDDConstants.h"
#include "Geometry/HGCalCommonData/interface/HGCalWaferIndex.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"

class HGCalWaferInFileTest : public edm::one::EDAnalyzer<> {
public:
  explicit HGCalWaferInFileTest(const edm::ParameterSet&);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  void beginJob() override {}
  void analyze(edm::Event const& iEvent, edm::EventSetup const&) override;
  void endJob() override {}

private:
  std::vector<std::string> getPoints(double xpos, double ypos, double delX, double delY, double rin, double rout, int lay, int waferU, int waferV);
  const std::string nameSense_, nameDetector_;
  const int verbosity_;
  const edm::ESGetToken<HGCalGeometry, IdealGeometryRecord> geomToken_;
};

HGCalWaferInFileTest::HGCalWaferInFileTest(const edm::ParameterSet& iC)
    : nameSense_(iC.getParameter<std::string>("NameSense")),
      nameDetector_(iC.getParameter<std::string>("NameDevice")),
      verbosity_(iC.getParameter<int>("Verbosity")),
      geomToken_(esConsumes<HGCalGeometry, IdealGeometryRecord>(edm::ESInputTag{"", nameSense_})) {
  std::cout << "Test wafer characteristics for " << nameDetector_ << " using constants of " << nameSense_
            << " with verbosity " << verbosity_ << std::endl;
}

void HGCalWaferInFileTest::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("NameSense", "HGCalEESensitive");
  desc.add<std::string>("NameDevice", "HGCal EE");
  desc.add<int>("Verbosity", 0);
  descriptions.add("hgcalEEWaferInFileTest", desc);
}

// ------------ method called to produce the data  ------------
void HGCalWaferInFileTest::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  const auto& geomR = iSetup.getData(geomToken_);
  const HGCalGeometry* geom = &geomR;
  const auto& hgdc = geom->topology().dddConstants();
  double delX = 0.5 * hgdc.getParameter()->waferSize_;
  double delY = 2.0 * delX / std::sqrt(3.0);

  std::cout << nameDetector_ << "\nCheck Wafers in file are all valid for " << nameDetector_ << std::endl << std::endl;
  if (hgdc.waferHexagon8()) {
    DetId::Detector det = (nameSense_ == "HGCalHESiliconSensitive") ? DetId::HGCalHSi : DetId::HGCalEE;
    static std::vector<std::string> types = {"F", "b", "g", "gm", "a", "d", "dm", "c", "X"};
    int layers = hgdc.layers(true);
    int layerf = hgdc.firstLayer();
    std::vector<int> miss(layers, 0);
    // See if all entries in the file are valid
    int bad1(0);
    for (unsigned int k = 0; k < hgdc.waferFileSize(); ++k) {
      int indx = hgdc.waferFileIndex(k);
      int layer = HGCalWaferIndex::waferLayer(indx);
      int waferU = HGCalWaferIndex::waferU(indx);
      int waferV = HGCalWaferIndex::waferV(indx);
      int type = std::get<0>(hgdc.waferFileInfo(k));
      HGCSiliconDetId id(det, 1, type, layer, waferU, waferV, 0, 0);
      if (!geom->topology().validModule(id, 3)) {
        int part = std::get<1>(hgdc.waferFileInfoFromIndex(indx));
        std::string typex = (part < static_cast<int>(types.size())) ? types[part] : "X";
        const auto& xy = hgdc.waferPosition(layer, waferU, waferV, true, true);
        const auto& rr = hgdc.rangeRLayer(layer, true);
        auto points = getPoints(xy.first, xy.second, delX, delY, rr.first, rr.second, layer, waferU, waferV);
        std::cout << "ID[" << k << "]: (" << (hgdc.getLayerOffset() + layer) << ", " << waferU << ", " << waferV << ", "
                  << typex << ") at (" << std::setprecision(4) << xy.first << ", " << xy.second << ", "
                  << hgdc.waferZ(layer, true) << ") not present with " << points.size() << " points:";
        for (auto point : points)
          std::cout << " " << point;
        std::cout << " in the region " << rr.first << ":" << rr.second << std::endl;
        ++bad1;
	if ((layer - layerf) < layers)
	  ++miss[layer - layerf];
      }
    }
    std::cout << "\n\nFinds " << bad1 << " invalid wafers among " << hgdc.waferFileSize() << " wafers in the list"
              << std::endl;
    for (unsigned int k = 0; k < miss.size(); ++k) {
      if (miss[k] > 0)
	std::cout << "Layer[" << k << ":" << (layerf + k) << "] " << miss[k] << std::endl;
    }
    std::cout << std::endl;

    // Now cross check the content (first type only)
    int allG(0), badT(0), badT1(0), badT2(0);
    for (unsigned int k = 0; k < hgdc.waferFileSize(); ++k) {
      int indx = hgdc.waferFileIndex(k);
      int type1 = std::get<0>(hgdc.waferFileInfo(k));
      int layer = HGCalWaferIndex::waferLayer(indx);
      int waferU = HGCalWaferIndex::waferU(indx);
      int waferV = HGCalWaferIndex::waferV(indx);
      int type2 = hgdc.waferType(layer, waferU, waferV, false);
      HGCSiliconDetId id(det, 1, type2, layer, waferU, waferV, 0, 0);
      if (geom->topology().validModule(id, 3)) {
        ++allG;
        bool typeOK = (type1 == type2);
        if (!typeOK) {
          ++badT;
          if (type1 == 0)
            ++badT1;
          else if (type2 == 0)
            ++badT2;
          const auto& xy = hgdc.waferPosition(layer, waferU, waferV, true, false);
          std::cout << "ID[" << k << "]: (" << (hgdc.getLayerOffset() + layer) << ", " << waferU << ", " << waferV
                    << ", " << type1 << ":" << type2 << ") at (" << std::setprecision(4) << xy.first << ", "
                    << xy.second << ", " << hgdc.waferZ(layer, true) << ") failure flag " << typeOK << std::endl;
        }
      }
    }
    std::cout << "\n\nFinds " << badT << "[" << badT1 << ":" << badT2 << "] mismatch in type among " << allG
              << " wafers with the same indices\n" << std::endl;

    // Now cross check the content (partial and orientation)
    int allX(0), badG(0), badP(0), badR(0);
    std::vector<int> wrongP(layers, 0), wrongR(layers, 0);
    for (unsigned int k = 0; k < hgdc.waferFileSize(); ++k) {
      int indx = hgdc.waferFileIndex(k);
      int part1 = std::get<1>(hgdc.waferFileInfo(k));
      int rotn1 = std::get<2>(hgdc.waferFileInfo(k));
      int layer = HGCalWaferIndex::waferLayer(indx);
      int waferU = HGCalWaferIndex::waferU(indx);
      int waferV = HGCalWaferIndex::waferV(indx);
      int type2 = hgdc.waferType(layer, waferU, waferV, false);
      HGCSiliconDetId id(det, 1, type2, layer, waferU, waferV, 0, 0);
      if (geom->topology().validModule(id, 3)) {
        ++allX;
        int part2 = hgdc.waferTypeRotation(id.layer(), id.waferU(), id.waferV(), false, false).first;
        int rotn2 = hgdc.waferTypeRotation(id.layer(), id.waferU(), id.waferV(), false, false).second;
        bool partOK = ((part1 == part2) || ((part1 == HGCalTypes::WaferFull) && (part2 == HGCalTypes::WaferOut)));
        bool rotnOK = ((rotn1 == rotn2) || (part1 == HGCalTypes::WaferFull) || (part2 == HGCalTypes::WaferFull));
        if (!partOK) {
          ++badP;
	  if ((layer - layerf) < layers)
	    ++wrongP[layer - layerf];
	}
        if (!rotnOK) {
          ++badR;
	  if ((layer - layerf) < layers)
	    ++wrongR[layer - layerf];
	}
        if ((!partOK) || (!rotnOK)) {
          ++badG;
          std::string partx1 = (part1 < static_cast<int>(types.size())) ? types[part1] : "X";
          std::string partx2 = (part2 < static_cast<int>(types.size())) ? types[part2] : "X";
          const auto& xy = hgdc.waferPosition(layer, waferU, waferV, true, false);
          const auto& rr = hgdc.rangeRLayer(layer, true);
          auto points = getPoints(xy.first, xy.second, delX, delY, rr.first, rr.second, layer, waferU, waferV);
          std::cout << "ID[" << k << "]: (" << (hgdc.getLayerOffset() + layer) << ", " << waferU << ", " << waferV << "," << type2 << ", " << partx1 << ":" << partx2 << ":" << part1 << ":" << part2 << ", " << rotn1 << ":" << rotn2 << ") at ("  << std::setprecision(4) << xy.first << ", " << xy.second << ", " << hgdc.waferZ(layer, true) << ") failure flag " << partOK << ":" << rotnOK << " with " << points.size() << " points:";
          for (auto point : points)
            std::cout << " " << point;
          std::cout << " in the region " << rr.first << ":" << rr.second << std::endl;
        }
      }
    }
    std::cout << "\n\nFinds " << badG << " (" << badP << ":" << badR << ") mismatch in partial|orientation among "
              << allX << " wafers with the same indices" << std::endl;
    for (int k = 0; k < layers; ++k) {
      if ((wrongP[k] > 0) || (wrongR[k] > 0))
	std::cout << "Layer[" << k << ":" << (layerf + k) << "] " << wrongP[k] << ":" << wrongR[k] << std::endl;
    }
    std::cout << std::endl;
  }
}

std::vector<std::string> HGCalWaferInFileTest::getPoints(double xpos, double ypos, double delX, double delY, double rin, double rout, int layer, int waferU, int waferV) {
  std::vector<std::string> points;
  static const int corners = 6;
  static const int base = 10;
  static const std::string c0[corners] = {"A0", "B0", "C0", "D0", "E0", "F0"};
  double dx0[corners] = {0.0, delX, delX, 0.0, -delX, -delX};
  double dy0[corners] = {-delY, -0.5 * delY, 0.5 * delY, delY, 0.5 * delY, -0.5 * delY};
  double xc[corners], yc[corners];
  int ncor(0), iok(0);
  for (int k = 0; k < corners; ++k) {
    xc[k] = xpos + dx0[k];
    yc[k] = ypos + dy0[k];
    double rpos = sqrt(xc[k] * xc[k] + yc[k] * yc[k]);
    if (rpos <= rout && rpos >= rin) {
      ++ncor;
      iok = iok * base + 1;
      points.emplace_back(c0[k]);
    } else {
      iok *= base;
    }
  }
  if (verbosity_ > 0) std::cout << "I/p " << layer << ":" << waferU << ":" << waferV << ":"  << xpos << ":" << ypos << ":" << delX << ":" << delY << ":" << rin << ":" << rout << " Corners " << ncor << " iok " << iok << std::endl;

  static const int parts = 3;
  static const std::string c1[parts] = {"A3", "A2", "A1"};
  double dx1[parts] = {0.75 * delX, 0.50 * delX, 0.25 * delX};
  double dy1[parts] = {-0.625 * delY, -0.75 * delY, -0.875 * delY};
  if ((((iok / 10000) % 10) == 1) && (((iok / 100000) % 10) == 0)) {
    for (int k = 0; k < parts; ++k) {
      double xc1 = xpos + dx1[k];
      double yc1 = ypos + dy1[k];
      double rpos = sqrt(xc1 * xc1 + yc1 * yc1);
      if (rpos <= rout && rpos >= rin) {
        points.emplace_back(c1[k]);
        break;
      }
    }
  }
  static const std::string c2[parts] = {"B3", "B2", "B1"};
  double dx2[parts] = {delX, delX, delX};
  double dy2[parts] = {0.5 * delY, 0.0, -0.5 * delY};
  if ((((iok / 1000) % 10) == 1) && (((iok / 10000) % 10) == 0)) {
    for (int k = 0; k < parts; ++k) {
      double xc1 = xpos + dx2[k];
      double yc1 = ypos + dy2[k];
      double rpos = sqrt(xc1 * xc1 + yc1 * yc1);
      if (rpos <= rout && rpos >= rin) {
        points.emplace_back(c2[k]);
        break;
      }
    }
  }
  static const std::string c3[parts] = {"C3", "C2", "C1"};
  double dx3[parts] = {0.25 * delX, 0.50 * delX, 0.75 * delX};
  double dy3[parts] = {0.875 * delY, 0.75 * delY, 0.625 * delY};
  if ((((iok / 100) % 10) == 1) && (((iok / 1000) % 10) == 0)) {
    for (int k = 0; k < parts; ++k) {
      double xc1 = xpos + dx3[k];
      double yc1 = ypos + dy3[k];
      double rpos = sqrt(xc1 * xc1 + yc1 * yc1);
      if (rpos <= rout && rpos >= rin) {
        points.emplace_back(c3[k]);
        break;
      }
    }
  }
  static const std::string c4[parts] = {"D3", "D2", "D1"};
  double dx4[parts] = {-0.75 * delX, -0.50 * delX, -0.25 * delX};
  double dy4[parts] = {0.625 * delY, 0.75 * delY, 0.875 * delY};
  if ((((iok / 10) % 10) == 1) && (((iok / 100) % 10) == 0)) {
    for (int k = 0; k < parts; ++k) {
      double xc1 = xpos + dx4[k];
      double yc1 = ypos + dy4[k];
      double rpos = sqrt(xc1 * xc1 + yc1 * yc1);
      if (rpos <= rout && rpos >= rin) {
        points.emplace_back(c4[k]);
        break;
      }
    }
  }
  static const std::string c5[parts] = {"E3", "E2", "E1"};
  double dx5[parts] = {-delX, -delX, -delX};
  double dy5[parts] = {-0.5 * delY, 0.0, 0.5 * delY};
  if ((((iok / 1) % 10) == 1) && (((iok / 10) % 10) == 0)) {
    for (int k = 0; k < parts; ++k) {
      double xc1 = xpos + dx5[k];
      double yc1 = ypos + dy5[k];
      double rpos = sqrt(xc1 * xc1 + yc1 * yc1);
      if (rpos <= rout && rpos >= rin) {
        points.emplace_back(c5[k]);
        break;
      }
    }
  }
  static const std::string c6[parts] = {"F3", "F2", "F1"};
  double dx6[parts] = {-0.25 * delX, -0.50 * delX, -0.75 * delX};
  double dy6[parts] = {-0.875 * delY, -0.75 * delY, -0.625 * delY};
  if ((((iok / 100000) % 10) == 1) && (((iok / 1) % 10) == 0)) {
    for (int k = 0; k < parts; ++k) {
      double xc1 = xpos + dx6[k];
      double yc1 = ypos + dy6[k];
      double rpos = sqrt(xc1 * xc1 + yc1 * yc1);
      if (rpos <= rout && rpos >= rin) {
        points.emplace_back(c6[k]);
        break;
      }
    }
  }
  return points;
}

// define this as a plug-in
DEFINE_FWK_MODULE(HGCalWaferInFileTest);
