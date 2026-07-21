#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"

static std::map<int, int> makeChargeMap() {
    std::map<int, int> m;
    m[1] = 999; m[2] = 999; m[3] = 999; m[4] = 999;
    m[21] = 999; m[23] = 999; m[24] = 999; m[2101] = 999;

    m[11] = -1; m[12] = 0; m[13] = -1; m[14] = 0;
    m[15] = -1; m[16] = 0; m[22] = 0;
    m[211] = 1; m[111] = 0;
    m[130] = 0; m[310] = 0; m[311] = 0;
    m[321] = 1; m[323] = 1;
    m[2212] = 1; m[2112] = 0;
    m[2114] = 0; m[2214] = 1; m[2224] = 2;
    m[3214] = 0; m[3112] = -1; m[3222] = 1;
    m[3212] = 0; m[3122] = 0;
    m[3312] = -1; m[3322] = 0; m[3224] = 1;
    m[3334] = -1; m[313] = 0;
    return m;
}

static int getCharge(int pdg, const std::map<int, int>& chargeMap) {
    auto it = chargeMap.find(std::abs(pdg));
    if (it != chargeMap.end()) {
        if (pdg > 0) return it->second;
        if (it->second == 999 || it->second == 0) return it->second;
        return -it->second;
    }
    return 999;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " [--NegativeID <status>] input1.hepmc [input2.hepmc ...] output.root"
                  << std::endl;
        return 1;
    }

    int negativeID = -999999;
    std::vector<std::string> positionalArgs;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--NegativeID" && i + 1 < argc) {
            negativeID = std::stoi(argv[++i]);
        } else {
            positionalArgs.push_back(arg);
        }
    }

    if (positionalArgs.size() < 2) {
        std::cerr << "Error: need at least one input file and one output file" << std::endl;
        return 1;
    }

    std::string outputFile = positionalArgs.back();
    std::vector<std::string> inputFiles(positionalArgs.begin(), positionalArgs.end() - 1);

    if (negativeID != -999999)
        std::cout << "NegativeID: status " << negativeID << " -> trackWeight = -1" << std::endl;

    auto chargeMap = makeChargeMap();

    TFile file(outputFile.c_str(), "RECREATE");
    TTree tree("Tree", "Tree");

    float EventWeight = 0;
    std::vector<float> genZPt, genZEta, genZY, genZPhi, genZMass;
    std::vector<float> genMuPt1, genMuPt2, genMuEta1, genMuEta2, genMuPhi1, genMuPhi2;
    std::vector<float> genMuDeta, genMuDphi, genMuDR, genMuDphiS;
    std::vector<float> v_trackPt, v_trackEta, v_trackPhi, v_trackY;
    std::vector<float> v_trackPDFId, v_trackWeight, v_trackResidualWeight;
    std::vector<float> v_trackMuTagged;
    std::vector<int> v_trackCharge, v_subevent;

    tree.Branch("EventWeight", &EventWeight);
    tree.Branch("genZPt", &genZPt);
    tree.Branch("genZEta", &genZEta);
    tree.Branch("genZY", &genZY);
    tree.Branch("genZPhi", &genZPhi);
    tree.Branch("genZMass", &genZMass);
    tree.Branch("genMuPt1", &genMuPt1);
    tree.Branch("genMuPt2", &genMuPt2);
    tree.Branch("genMuEta1", &genMuEta1);
    tree.Branch("genMuEta2", &genMuEta2);
    tree.Branch("genMuPhi1", &genMuPhi1);
    tree.Branch("genMuPhi2", &genMuPhi2);
    tree.Branch("genMuDeta", &genMuDeta);
    tree.Branch("genMuDphi", &genMuDphi);
    tree.Branch("genMuDR", &genMuDR);
    tree.Branch("genMuDphiS", &genMuDphiS);
    tree.Branch("trackPt", &v_trackPt);
    tree.Branch("trackEta", &v_trackEta);
    tree.Branch("trackPhi", &v_trackPhi);
    tree.Branch("trackY", &v_trackY);
    tree.Branch("trackPDFId", &v_trackPDFId);
    tree.Branch("trackWeight", &v_trackWeight);
    tree.Branch("trackResidualWeight", &v_trackResidualWeight);
    tree.Branch("trackMuTagged", &v_trackMuTagged);
    tree.Branch("trackCharge", &v_trackCharge);
    tree.Branch("subevent", &v_subevent);

    struct Particle {
        int pdg, status;
        double px, py, pz, energy, mass;
    };

    int totalEvents = 0;
    bool inEvent = false;
    std::vector<Particle> particles;

    auto flushEvent = [&]() {
        if (!inEvent) return;

        // Step 1: Fill ALL particles into track arrays
        v_trackPt.clear(); v_trackEta.clear(); v_trackPhi.clear(); v_trackY.clear();
        v_trackPDFId.clear(); v_trackWeight.clear(); v_trackResidualWeight.clear();
        v_trackMuTagged.clear(); v_trackCharge.clear(); v_subevent.clear();

        for (auto& p : particles) {
            double pt = std::sqrt(p.px * p.px + p.py * p.py);
            double pmag = std::sqrt(p.px * p.px + p.py * p.py + p.pz * p.pz);
            double eta = (pmag > 1e-10) ? 0.5 * std::log((pmag + p.pz) / (pmag - p.pz)) : 0;
            double rap = (p.energy > std::abs(p.pz)) ?
                0.5 * std::log((p.energy + p.pz) / (p.energy - p.pz)) : 0;

            double phi = 0;
            if (p.px == 0 && p.py > 0)       phi = M_PI / 2;
            else if (p.px == 0 && p.py < 0)  phi = -M_PI / 2;
            else                              phi = std::atan(p.py / p.px);
            if (p.px < 0)                     phi = phi + M_PI;
            if (phi > M_PI)                   phi = phi - 2 * M_PI;

            double weight = 0;
            if (p.status == 1) weight = 1;
            if (p.status == negativeID) weight = -1;

            v_trackPt.push_back(pt);
            v_trackEta.push_back(eta);
            v_trackPhi.push_back(phi);
            v_trackY.push_back(rap);
            v_trackPDFId.push_back(p.pdg);
            v_trackWeight.push_back(weight);
            v_trackResidualWeight.push_back(1);
            v_trackMuTagged.push_back(0);
            v_trackCharge.push_back(999);
            v_subevent.push_back(0);
        }

        // Step 2: SetCharge
        for (size_t i = 0; i < v_trackPt.size(); i++) {
            if (v_trackWeight[i] == 0) continue;
            int id = (int)v_trackPDFId[i];
            int charge = getCharge(id, chargeMap);
            v_trackCharge[i] = charge;
        }

        // Step 3: Tag muons
        for (size_t i = 0; i < v_trackPt.size(); i++) {
            int id = (int)v_trackPDFId[i];
            if (id == 13 || id == -13)
                v_trackMuTagged[i] = 1;
        }

        // Step 4: ZeroNeutrals (ChargedOnly=true)
        for (size_t i = 0; i < v_trackPt.size(); i++) {
            if (v_trackCharge[i] == 0)
                v_trackWeight[i] = 0;
        }

        // Step 5: Z reconstruction with CMS-style cuts
        genZPt.clear(); genZEta.clear(); genZY.clear(); genZPhi.clear(); genZMass.clear();
        genMuPt1.clear(); genMuPt2.clear(); genMuEta1.clear(); genMuEta2.clear();
        genMuPhi1.clear(); genMuPhi2.clear();
        genMuDeta.clear(); genMuDphi.clear(); genMuDR.clear(); genMuDphiS.clear();

        std::vector<int> muons, antiMuons;
        for (size_t i = 0; i < v_trackPt.size(); i++) {
            int id = (int)v_trackPDFId[i];
            if (id != 13 && id != -13) continue;
            if (v_trackWeight[i] == 0) continue;
            if (std::abs(v_trackEta[i]) > 2.4) continue;
            if (v_trackPt[i] < 20) continue;

            if (id == 13)  muons.push_back(i);
            if (id == -13) antiMuons.push_back(i);
        }

        for (int i1 : muons) {
            TLorentzVector Mu1;
            Mu1.SetPtEtaPhiM(v_trackPt[i1], v_trackEta[i1], v_trackPhi[i1], 0.105);

            for (int i2 : antiMuons) {
                TLorentzVector Mu2;
                Mu2.SetPtEtaPhiM(v_trackPt[i2], v_trackEta[i2], v_trackPhi[i2], 0.105);

                TLorentzVector VZ = Mu1 + Mu2;

                if (VZ.M() < 60 || VZ.M() > 120) continue;
                if (std::abs(VZ.Rapidity()) > 2.4) continue;

                genZPt.push_back(VZ.Pt());
                genZEta.push_back(VZ.Eta());
                genZY.push_back(VZ.Rapidity());
                genZPhi.push_back(VZ.Phi());
                genZMass.push_back(VZ.M());

                genMuPt1.push_back(Mu1.Pt());
                genMuPt2.push_back(Mu2.Pt());
                genMuEta1.push_back(Mu1.Eta());
                genMuEta2.push_back(Mu2.Eta());
                genMuPhi1.push_back(Mu1.Phi());
                genMuPhi2.push_back(Mu2.Phi());

                double deta = Mu1.Eta() - Mu2.Eta();
                double dphi = Mu1.Phi() - Mu2.Phi();
                if (dphi < -M_PI) dphi += 2 * M_PI;
                if (dphi > +M_PI) dphi -= 2 * M_PI;
                genMuDeta.push_back(deta);
                genMuDphi.push_back(dphi);
                genMuDR.push_back(std::sqrt(deta * deta + dphi * dphi));

                double dphiS = std::tan((M_PI - dphi) / 2)
                    * std::sqrt(1 - std::tanh(deta / 2) * std::tanh(deta / 2));
                genMuDphiS.push_back(dphiS);
            }
        }

        tree.Fill();
        totalEvents++;
    };

    for (auto& fname : inputFiles) {
        std::ifstream fin(fname);
        if (!fin.is_open()) {
            std::cerr << "Cannot open " << fname << std::endl;
            continue;
        }
        std::cout << "Processing " << fname << "..." << std::endl;

        std::string line;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            char type = line[0];

            if (type == 'E') {
                flushEvent();
                inEvent = true;
                particles.clear();

                std::vector<std::string> tokens;
                std::istringstream ss(line);
                std::string tok;
                while (ss >> tok) tokens.push_back(tok);
                EventWeight = std::stof(tokens[tokens.size() - 1]);
            } else if (type == 'P') {
                std::istringstream ss(line);
                std::string tag;
                int barcode, pdg;
                double px, py, pz, energy, mass;
                int status;
                ss >> tag >> barcode >> pdg >> px >> py >> pz >> energy >> mass >> status;
                particles.push_back({pdg, status, px, py, pz, energy, mass});
            }
        }
        fin.close();
    }
    flushEvent();

    tree.Write();
    file.Close();
    std::cout << "Wrote " << totalEvents << " events to " << outputFile << std::endl;
    return 0;
}
