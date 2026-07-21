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

TH1D* makeRatio(TH1D* hNum, TH1D* hDen, const char* name) {
    TH1D* hRat = (TH1D*)hNum->Clone(name);
    hRat->Divide(hDen);
    return hRat;
}

void drawWithRatio(TCanvas& c, int padIdx,
                   TH1D* hOur, TH1D* hRef,
                   const char* xtitle, bool logX, bool logY,
                   const char* saveName, const std::string& outDir) {
    // Main pad
    c.cd(padIdx);
    TPad* pMain = new TPad(Form("main_%d", padIdx), "", 0, 0.3, 1, 1);
    pMain->SetBottomMargin(0.02);
    pMain->SetLeftMargin(0.15);
    pMain->SetRightMargin(0.05);
    if (logX) pMain->SetLogx();
    if (logY) pMain->SetLogy();
    pMain->Draw();
    pMain->cd();

    double maxY = std::max(hOur->GetMaximum(), hRef->GetMaximum()) * 1.5;
    double minY = logY ? maxY * 1e-6 : 0;
    hRef->GetYaxis()->SetRangeUser(minY, maxY);
    hRef->GetXaxis()->SetLabelSize(0);
    hRef->GetXaxis()->SetTitleSize(0);
    hRef->GetYaxis()->SetTitle("dN/dx (weighted)");
    hRef->GetYaxis()->SetTitleSize(0.06);
    hRef->GetYaxis()->SetTitleOffset(1.0);
    hRef->GetYaxis()->SetLabelSize(0.05);
    hRef->Draw("E");
    hOur->Draw("E SAME");

    TLegend* leg = new TLegend(0.50, 0.72, 0.93, 0.92);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.05);
    leg->AddEntry(hOur, "JEWEL 2.2.0 (this work)", "lep");
    leg->AddEntry(hRef, "Reference v7", "lep");
    leg->Draw();

    // Chi2
    double chi2 = 0;
    int ndf = 0;
    for (int b = 1; b <= hOur->GetNbinsX(); b++) {
        double vo = hOur->GetBinContent(b), eo = hOur->GetBinError(b);
        double vr = hRef->GetBinContent(b), er = hRef->GetBinError(b);
        double e2 = eo * eo + er * er;
        if (e2 > 0 && (vo > 0 || vr > 0)) {
            chi2 += (vo - vr) * (vo - vr) / e2;
            ndf++;
        }
    }
    TLatex tex;
    tex.SetNDC();
    tex.SetTextSize(0.05);
    if (ndf > 0)
        tex.DrawLatex(0.18, 0.85, Form("#chi^{2}/ndf = %.1f/%d = %.2f", chi2, ndf, chi2 / ndf));

    // Ratio pad
    c.cd(padIdx);
    TPad* pRat = new TPad(Form("ratio_%d", padIdx), "", 0, 0, 1, 0.3);
    pRat->SetTopMargin(0.02);
    pRat->SetBottomMargin(0.35);
    pRat->SetLeftMargin(0.15);
    pRat->SetRightMargin(0.05);
    if (logX) pRat->SetLogx();
    pRat->Draw();
    pRat->cd();

    TH1D* hRat = makeRatio(hOur, hRef, Form("ratio_%s", saveName));
    hRat->SetLineColor(kBlack);
    hRat->SetMarkerColor(kBlack);
    hRat->SetMarkerStyle(20);
    hRat->SetMarkerSize(0.5);
    hRat->GetYaxis()->SetRangeUser(0.0, 2.0);
    hRat->GetYaxis()->SetTitle("Ratio");
    hRat->GetYaxis()->SetTitleSize(0.12);
    hRat->GetYaxis()->SetTitleOffset(0.45);
    hRat->GetYaxis()->SetLabelSize(0.10);
    hRat->GetYaxis()->SetNdivisions(505);
    hRat->GetXaxis()->SetTitle(xtitle);
    hRat->GetXaxis()->SetTitleSize(0.14);
    hRat->GetXaxis()->SetTitleOffset(0.9);
    hRat->GetXaxis()->SetLabelSize(0.10);
    hRat->Draw("E");

    TLine* line = new TLine(hRat->GetXaxis()->GetXmin(), 1.0,
                            hRat->GetXaxis()->GetXmax(), 1.0);
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
    hOur->Draw("E SAME");
    leg->Draw();
    if (ndf > 0)
        tex.DrawLatex(0.18, 0.85, Form("#chi^{2}/ndf = %.1f/%d = %.2f", chi2, ndf, chi2 / ndf));

    cSingle.cd();
    TPad* pR2 = new TPad("pr2", "", 0, 0, 1, 0.3);
    pR2->SetTopMargin(0.02);
    pR2->SetBottomMargin(0.35);
    pR2->SetLeftMargin(0.15);
    pR2->SetRightMargin(0.05);
    if (logX) pR2->SetLogx();
    pR2->Draw();
    pR2->cd();
    hRat->Draw("E");
    line->Draw();

    cSingle.SaveAs((outDir + "/" + saveName + ".png").c_str());
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " our.root ref.root outputDir/" << std::endl;
        return 1;
    }

    std::string ourFile = argv[1];
    std::string refFile = argv[2];
    std::string outDir = argv[3];
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

    Histos hOur, hRef;
    hOur.hZPt = new TH1D("hOur_ZPt", "", nZPtBins, zPtBins);
    hOur.hZEta = new TH1D("hOur_ZEta", "", 30, -3.0, 3.0);
    hOur.hTrkPt = new TH1D("hOur_TrkPt", "", nTrkPtBins, trkPtBins);
    hOur.hTrkEta = new TH1D("hOur_TrkEta", "", 30, -2.4, 2.4);

    hRef.hZPt = new TH1D("hRef_ZPt", "", nZPtBins, zPtBins);
    hRef.hZEta = new TH1D("hRef_ZEta", "", 30, -3.0, 3.0);
    hRef.hTrkPt = new TH1D("hRef_TrkPt", "", nTrkPtBins, trkPtBins);
    hRef.hTrkEta = new TH1D("hRef_TrkEta", "", 30, -2.4, 2.4);

    hOur.hZPt->Sumw2(); hOur.hZEta->Sumw2();
    hOur.hTrkPt->Sumw2(); hOur.hTrkEta->Sumw2();
    hRef.hZPt->Sumw2(); hRef.hZEta->Sumw2();
    hRef.hTrkPt->Sumw2(); hRef.hTrkEta->Sumw2();

    TFile* fOur = TFile::Open(ourFile.c_str());
    TTree* tOur = (TTree*)fOur->Get("Tree");
    std::cout << "Our file: " << tOur->GetEntries() << " events" << std::endl;
    fillFromTree(tOur, hOur);
    fOur->Close();

    TFile* fRef = TFile::Open(refFile.c_str());
    TTree* tRef = (TTree*)fRef->Get("Tree");
    std::cout << "Reference file: " << tRef->GetEntries() << " events" << std::endl;
    fillFromTree(tRef, hRef);
    fRef->Close();

    // Normalize by integral (shape comparison, preserving weighted errors)
    auto normalize = [](TH1D* h) {
        double integral = h->Integral("width");
        if (integral > 0) h->Scale(1.0 / integral);
    };
    normalize(hOur.hZPt); normalize(hRef.hZPt);
    normalize(hOur.hZEta); normalize(hRef.hZEta);
    normalize(hOur.hTrkPt); normalize(hRef.hTrkPt);
    normalize(hOur.hTrkEta); normalize(hRef.hTrkEta);

    // Style
    auto style = [](TH1D* h, int color, int marker) {
        h->SetLineColor(color);
        h->SetMarkerColor(color);
        h->SetMarkerStyle(marker);
        h->SetMarkerSize(0.6);
        h->SetLineWidth(2);
    };
    style(hOur.hZPt, kRed, 20); style(hOur.hZEta, kRed, 20);
    style(hOur.hTrkPt, kRed, 20); style(hOur.hTrkEta, kRed, 20);
    style(hRef.hZPt, kBlue, 24); style(hRef.hZEta, kBlue, 24);
    style(hRef.hTrkPt, kBlue, 24); style(hRef.hTrkEta, kBlue, 24);

    TCanvas c("c", "JEWEL 2.2.0 Validation", 1400, 1100);
    c.Divide(2, 2);

    drawWithRatio(c, 1, hOur.hZPt, hRef.hZPt, "Z p_{T} [GeV]", true, true, "z_pt", outDir);
    drawWithRatio(c, 2, hOur.hZEta, hRef.hZEta, "Z #eta", false, false, "z_eta", outDir);
    drawWithRatio(c, 3, hOur.hTrkPt, hRef.hTrkPt, "Track p_{T} [GeV]", true, true, "track_pt", outDir);
    drawWithRatio(c, 4, hOur.hTrkEta, hRef.hTrkEta, "Track #eta", false, false, "track_eta", outDir);

    c.SaveAs((outDir + "/validation_overlay.pdf").c_str());
    c.SaveAs((outDir + "/validation_overlay.png").c_str());

    std::cout << "Plots saved to " << outDir << std::endl;
    return 0;
}
