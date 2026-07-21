#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TPad.h"
#include "TLine.h"
#include "TSystem.h"

struct Histos {
    TH1D* hZPt;
    TH1D* hZEta;
    TH1D* hTrkPt;
    TH1D* hTrkEta;
};

void fillFromTree(TTree* tree, Histos& h) {
    float EventWeight;
    std::vector<float>* genZPt = nullptr;
    std::vector<float>* genZEta = nullptr;
    std::vector<float>* trackPt = nullptr;
    std::vector<float>* trackEta = nullptr;
    std::vector<float>* trackWeight = nullptr;
    std::vector<int>* trackCharge = nullptr;

    tree->SetBranchAddress("EventWeight", &EventWeight);
    tree->SetBranchAddress("genZPt", &genZPt);
    tree->SetBranchAddress("genZEta", &genZEta);
    tree->SetBranchAddress("trackPt", &trackPt);
    tree->SetBranchAddress("trackEta", &trackEta);
    tree->SetBranchAddress("trackWeight", &trackWeight);
    tree->SetBranchAddress("trackCharge", &trackCharge);

    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);

        if (genZPt && genZPt->size() > 0 && genZPt->at(0) > 0) {
            h.hZPt->Fill(genZPt->at(0), EventWeight);
            h.hZEta->Fill(genZEta->at(0), EventWeight);
        }

        if (trackPt && trackWeight && trackCharge) {
            for (size_t t = 0; t < trackPt->size(); t++) {
                if (trackWeight->at(t) < 0.5) continue;
                if (trackCharge->at(t) == 999) continue;
                if (std::abs(trackEta->at(t)) > 2.4) continue;
                if (trackPt->at(t) < 0.5) continue;
                h.hTrkPt->Fill(trackPt->at(t), EventWeight);
                h.hTrkEta->Fill(trackEta->at(t), EventWeight);
            }
        }
    }
}

double calcChi2(TH1D* h1, TH1D* h2, int& ndf) {
    double chi2 = 0;
    ndf = 0;
    for (int b = 1; b <= h1->GetNbinsX(); b++) {
        double v1 = h1->GetBinContent(b), e1 = h1->GetBinError(b);
        double v2 = h2->GetBinContent(b), e2 = h2->GetBinError(b);
        double e2sum = e1 * e1 + e2 * e2;
        if (e2sum > 0 && (v1 > 0 || v2 > 0)) {
            chi2 += (v1 - v2) * (v1 - v2) / e2sum;
            ndf++;
        }
    }
    return chi2;
}

void draw3WayWithRatio(TCanvas& c, int padIdx,
                       TH1D* h240, TH1D* h220, TH1D* hRef,
                       const char* xtitle, bool logX, bool logY,
                       const char* saveName, const std::string& outDir) {
    c.cd(padIdx);
    TPad* pMain = new TPad(Form("main_%d", padIdx), "", 0, 0.3, 1, 1);
    pMain->SetBottomMargin(0.02);
    pMain->SetLeftMargin(0.15);
    pMain->SetRightMargin(0.05);
    if (logX) pMain->SetLogx();
    if (logY) pMain->SetLogy();
    pMain->Draw();
    pMain->cd();

    double maxY = std::max({h240->GetMaximum(), h220->GetMaximum(), hRef->GetMaximum()}) * 1.5;
    double minY = logY ? maxY * 1e-6 : 0;
    hRef->GetYaxis()->SetRangeUser(minY, maxY);
    hRef->GetXaxis()->SetLabelSize(0);
    hRef->GetXaxis()->SetTitleSize(0);
    hRef->GetYaxis()->SetTitle("dN/dx (weighted)");
    hRef->GetYaxis()->SetTitleSize(0.06);
    hRef->GetYaxis()->SetTitleOffset(1.0);
    hRef->GetYaxis()->SetLabelSize(0.05);
    hRef->Draw("E");
    h220->Draw("E SAME");
    h240->Draw("E SAME");

    TLegend* leg = new TLegend(0.42, 0.65, 0.93, 0.92);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.045);
    leg->AddEntry(h240, "JEWEL 2.4.0-2D pp", "lep");
    leg->AddEntry(h220, "JEWEL 2.2.0 pp", "lep");
    leg->AddEntry(hRef, "Reference v9", "lep");
    leg->Draw();

    int ndf240 = 0, ndf220 = 0;
    double chi2_240 = calcChi2(h240, hRef, ndf240);
    double chi2_220 = calcChi2(h220, hRef, ndf220);
    TLatex tex;
    tex.SetNDC();
    tex.SetTextSize(0.04);
    if (ndf240 > 0) {
        tex.SetTextColor(kRed);
        tex.DrawLatex(0.17, 0.88, Form("2.4.0: #chi^{2}/ndf = %.1f/%d = %.2f", chi2_240, ndf240, chi2_240 / ndf240));
    }
    if (ndf220 > 0) {
        tex.SetTextColor(kBlue);
        tex.DrawLatex(0.17, 0.82, Form("2.2.0: #chi^{2}/ndf = %.1f/%d = %.2f", chi2_220, ndf220, chi2_220 / ndf220));
    }

    // Ratio pad (ratio to v7 reference)
    c.cd(padIdx);
    TPad* pRat = new TPad(Form("ratio_%d", padIdx), "", 0, 0, 1, 0.3);
    pRat->SetTopMargin(0.02);
    pRat->SetBottomMargin(0.35);
    pRat->SetLeftMargin(0.15);
    pRat->SetRightMargin(0.05);
    if (logX) pRat->SetLogx();
    pRat->Draw();
    pRat->cd();

    TH1D* hRat240 = (TH1D*)h240->Clone(Form("ratio240_%s", saveName));
    hRat240->Divide(hRef);
    hRat240->SetLineColor(kRed);
    hRat240->SetMarkerColor(kRed);
    hRat240->SetMarkerStyle(20);
    hRat240->SetMarkerSize(0.5);

    TH1D* hRat220 = (TH1D*)h220->Clone(Form("ratio220_%s", saveName));
    hRat220->Divide(hRef);
    hRat220->SetLineColor(kBlue);
    hRat220->SetMarkerColor(kBlue);
    hRat220->SetMarkerStyle(24);
    hRat220->SetMarkerSize(0.5);

    hRat240->GetYaxis()->SetRangeUser(0.0, 2.0);
    hRat240->GetYaxis()->SetTitle("Ratio to v9");
    hRat240->GetYaxis()->SetTitleSize(0.12);
    hRat240->GetYaxis()->SetTitleOffset(0.45);
    hRat240->GetYaxis()->SetLabelSize(0.10);
    hRat240->GetYaxis()->SetNdivisions(505);
    hRat240->GetXaxis()->SetTitle(xtitle);
    hRat240->GetXaxis()->SetTitleSize(0.14);
    hRat240->GetXaxis()->SetTitleOffset(0.9);
    hRat240->GetXaxis()->SetLabelSize(0.10);
    hRat240->Draw("E");
    hRat220->Draw("E SAME");

    TLine* line = new TLine(hRat240->GetXaxis()->GetXmin(), 1.0,
                            hRat240->GetXaxis()->GetXmax(), 1.0);
    line->SetLineStyle(2);
    line->SetLineColor(kGray + 2);
    line->Draw();

    // Save individual
    TCanvas cSingle(Form("c_%s", saveName), "", 700, 700);
    TPad* pM2 = new TPad("pm2", "", 0, 0.3, 1, 1);
    pM2->SetBottomMargin(0.02);
    pM2->SetLeftMargin(0.15);
    pM2->SetRightMargin(0.05);
    if (logX) pM2->SetLogx();
    if (logY) pM2->SetLogy();
    pM2->Draw();
    pM2->cd();
    hRef->Draw("E");
    h220->Draw("E SAME");
    h240->Draw("E SAME");
    leg->Draw();
    tex.SetTextColor(kRed);
    if (ndf240 > 0)
        tex.DrawLatex(0.17, 0.88, Form("2.4.0: #chi^{2}/ndf = %.1f/%d = %.2f", chi2_240, ndf240, chi2_240 / ndf240));
    tex.SetTextColor(kBlue);
    if (ndf220 > 0)
        tex.DrawLatex(0.17, 0.82, Form("2.2.0: #chi^{2}/ndf = %.1f/%d = %.2f", chi2_220, ndf220, chi2_220 / ndf220));

    cSingle.cd();
    TPad* pR2 = new TPad("pr2", "", 0, 0, 1, 0.3);
    pR2->SetTopMargin(0.02);
    pR2->SetBottomMargin(0.35);
    pR2->SetLeftMargin(0.15);
    pR2->SetRightMargin(0.05);
    if (logX) pR2->SetLogx();
    pR2->Draw();
    pR2->cd();
    hRat240->Draw("E");
    hRat220->Draw("E SAME");
    line->Draw();

    cSingle.SaveAs((outDir + "/" + saveName + ".png").c_str());
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " jewel240.root jewel220.root ref_v7.root outputDir/" << std::endl;
        return 1;
    }

    std::string file240 = argv[1];
    std::string file220 = argv[2];
    std::string fileRef = argv[3];
    std::string outDir = argv[4];
    gSystem->mkdir(outDir.c_str(), true);

    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);

    const int nZPtBins = 30;
    double zPtBins[nZPtBins + 1];
    for (int i = 0; i <= nZPtBins; i++)
        zPtBins[i] = 1.0 * std::pow(1000.0 / 1.0, (double)i / nZPtBins);

    const int nTrkPtBins = 25;
    double trkPtBins[nTrkPtBins + 1];
    for (int i = 0; i <= nTrkPtBins; i++)
        trkPtBins[i] = 0.5 * std::pow(50.0 / 0.5, (double)i / nTrkPtBins);

    auto makeHistos = [&](const char* prefix) -> Histos {
        Histos h;
        h.hZPt = new TH1D(Form("%s_ZPt", prefix), "", nZPtBins, zPtBins);
        h.hZEta = new TH1D(Form("%s_ZEta", prefix), "", 30, -3.0, 3.0);
        h.hTrkPt = new TH1D(Form("%s_TrkPt", prefix), "", nTrkPtBins, trkPtBins);
        h.hTrkEta = new TH1D(Form("%s_TrkEta", prefix), "", 30, -2.4, 2.4);
        h.hZPt->Sumw2(); h.hZEta->Sumw2();
        h.hTrkPt->Sumw2(); h.hTrkEta->Sumw2();
        return h;
    };

    Histos h240 = makeHistos("h240");
    Histos h220 = makeHistos("h220");
    Histos hRef = makeHistos("hRef");

    auto loadFile = [](const char* path, Histos& h, const char* label) {
        TFile* f = TFile::Open(path);
        TTree* t = (TTree*)f->Get("Tree");
        std::cout << label << ": " << t->GetEntries() << " events" << std::endl;
        fillFromTree(t, h);
        f->Close();
    };

    loadFile(file240.c_str(), h240, "JEWEL 2.4.0-2D");
    loadFile(file220.c_str(), h220, "JEWEL 2.2.0");
    loadFile(fileRef.c_str(), hRef, "Reference v9");

    auto normalize = [](TH1D* h) {
        double integral = h->Integral("width");
        if (integral > 0) h->Scale(1.0 / integral);
    };
    normalize(h240.hZPt); normalize(h220.hZPt); normalize(hRef.hZPt);
    normalize(h240.hZEta); normalize(h220.hZEta); normalize(hRef.hZEta);
    normalize(h240.hTrkPt); normalize(h220.hTrkPt); normalize(hRef.hTrkPt);
    normalize(h240.hTrkEta); normalize(h220.hTrkEta); normalize(hRef.hTrkEta);

    auto style = [](Histos& h, int color, int marker) {
        for (auto* hist : {h.hZPt, h.hZEta, h.hTrkPt, h.hTrkEta}) {
            hist->SetLineColor(color);
            hist->SetMarkerColor(color);
            hist->SetMarkerStyle(marker);
            hist->SetMarkerSize(0.6);
            hist->SetLineWidth(2);
        }
    };
    style(h240, kRed, 20);
    style(h220, kBlue, 24);
    style(hRef, kBlack, 25);

    TCanvas c("c", "JEWEL 2.4.0 vs 2.2.0 vs v7", 1400, 1100);
    c.Divide(2, 2);

    draw3WayWithRatio(c, 1, h240.hZPt, h220.hZPt, hRef.hZPt,
                      "Z p_{T} [GeV]", true, true, "z_pt", outDir);
    draw3WayWithRatio(c, 2, h240.hZEta, h220.hZEta, hRef.hZEta,
                      "Z #eta", false, false, "z_eta", outDir);
    draw3WayWithRatio(c, 3, h240.hTrkPt, h220.hTrkPt, hRef.hTrkPt,
                      "Track p_{T} [GeV]", true, true, "track_pt", outDir);
    draw3WayWithRatio(c, 4, h240.hTrkEta, h220.hTrkEta, hRef.hTrkEta,
                      "Track #eta", false, false, "track_eta", outDir);

    c.SaveAs((outDir + "/validation_3way.pdf").c_str());
    c.SaveAs((outDir + "/validation_3way.png").c_str());

    std::cout << "Plots saved to " << outDir << std::endl;
    return 0;
}
