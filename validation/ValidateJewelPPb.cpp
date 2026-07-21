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

struct Counts {
    double nZuw;
    double nZw;
    double nEvtW;
    Long64_t nEvt;
};

Counts fillFromTree(TTree* tree, Histos& h, bool hasRecoils = false) {
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

    Counts c = {0, 0, 0, tree->GetEntries()};
    for (Long64_t i = 0; i < c.nEvt; i++) {
        tree->GetEntry(i);
        c.nEvtW += EventWeight;

        if (genZPt && genZPt->size() > 0 && genZPt->at(0) > 0) {
            h.hZPt->Fill(genZPt->at(0), EventWeight);
            h.hZEta->Fill(genZEta->at(0), EventWeight);
            c.nZuw += 1;
            c.nZw += EventWeight;
        }

        if (trackPt && trackWeight && trackCharge) {
            for (size_t t = 0; t < trackPt->size(); t++) {
                float tw = trackWeight->at(t);
                if (hasRecoils) {
                    if (tw == 0) continue;
                } else {
                    if (tw < 0.5) continue;
                }
                if (trackCharge->at(t) == 999) continue;
                if (std::abs(trackEta->at(t)) > 2.4) continue;
                if (trackPt->at(t) < 0.5) continue;
                double holeCorr = hasRecoils ? (1 - 0.33 * (tw < 0)) : 1.0;
                h.hTrkPt->Fill(trackPt->at(t), EventWeight * holeCorr);
                h.hTrkEta->Fill(trackEta->at(t), EventWeight * holeCorr);
            }
        }
    }
    return c;
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

void drawWithRatio(TCanvas& c, int padIdx,
                   TH1D* hPPb, TH1D* hPP,
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

    double maxY = std::max(hPPb->GetMaximum(), hPP->GetMaximum()) * 1.5;
    double minY = logY ? maxY * 1e-6 : 0;
    hPP->GetYaxis()->SetRangeUser(minY, maxY);
    hPP->GetXaxis()->SetLabelSize(0);
    hPP->GetXaxis()->SetTitleSize(0);
    hPP->GetYaxis()->SetTitle("dN/dx (weighted)");
    hPP->GetYaxis()->SetTitleSize(0.06);
    hPP->GetYaxis()->SetTitleOffset(1.0);
    hPP->GetYaxis()->SetLabelSize(0.05);
    hPP->Draw("E");
    hPPb->Draw("E SAME");

    TLegend* leg = new TLegend(0.42, 0.70, 0.93, 0.92);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.045);
    leg->AddEntry(hPPb, "JEWEL 2.4.0-2D pPb (hydro)", "lep");
    leg->AddEntry(hPP, "JEWEL 2.4.0-2D pp (vacuum)", "lep");
    leg->Draw();

    int ndf = 0;
    double chi2 = calcChi2(hPPb, hPP, ndf);
    TLatex tex;
    tex.SetNDC();
    tex.SetTextSize(0.04);
    tex.SetTextColor(kBlack);
    if (ndf > 0)
        tex.DrawLatex(0.17, 0.88, Form("#chi^{2}/ndf = %.1f/%d = %.2f", chi2, ndf, chi2 / (double)ndf));

    c.cd(padIdx);
    TPad* pRat = new TPad(Form("ratio_%d", padIdx), "", 0, 0, 1, 0.3);
    pRat->SetTopMargin(0.02);
    pRat->SetBottomMargin(0.35);
    pRat->SetLeftMargin(0.15);
    pRat->SetRightMargin(0.05);
    if (logX) pRat->SetLogx();
    pRat->Draw();
    pRat->cd();

    TH1D* hRat = (TH1D*)hPPb->Clone(Form("ratio_%s", saveName));
    hRat->Divide(hPP);
    hRat->SetLineColor(kBlack);
    hRat->SetMarkerColor(kBlack);
    hRat->SetMarkerStyle(20);
    hRat->SetMarkerSize(0.5);

    hRat->GetYaxis()->SetRangeUser(0.0, 2.0);
    hRat->GetYaxis()->SetTitle("pPb / pp");
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

    TCanvas cSingle(Form("c_%s", saveName), "", 700, 700);
    TPad* pM2 = new TPad("pm2", "", 0, 0.3, 1, 1);
    pM2->SetBottomMargin(0.02);
    pM2->SetLeftMargin(0.15);
    pM2->SetRightMargin(0.05);
    if (logX) pM2->SetLogx();
    if (logY) pM2->SetLogy();
    pM2->Draw();
    pM2->cd();
    hPP->Draw("E");
    hPPb->Draw("E SAME");
    leg->Draw();
    if (ndf > 0)
        tex.DrawLatex(0.17, 0.88, Form("#chi^{2}/ndf = %.1f/%d = %.2f", chi2, ndf, chi2 / (double)ndf));

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
        std::cerr << "Usage: " << argv[0] << " jewel_pPb.root jewel_pp.root outputDir/" << std::endl;
        return 1;
    }

    std::string filePPb = argv[1];
    std::string filePP = argv[2];
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

    Histos hPPb = makeHistos("hPPb");
    Histos hPP = makeHistos("hPP");

    Histos hPPb_uw = makeHistos("hPPb_uw");
    Histos hPP_uw = makeHistos("hPP_uw");

    auto loadFile = [](const char* path, Histos& h, const char* label, bool hasRecoils) -> Counts {
        TFile* f = TFile::Open(path);
        TTree* t = (TTree*)f->Get("Tree");
        std::cout << label << ": " << t->GetEntries() << " events" << std::endl;
        Counts c = fillFromTree(t, h, hasRecoils);
        f->Close();
        return c;
    };

    Counts cPPb = loadFile(filePPb.c_str(), hPPb, "pPb (hydro)", true);
    Counts cPP = loadFile(filePP.c_str(), hPP, "pp (vacuum)", false);

    std::cout << "pPb: nZ(uw)=" << cPPb.nZuw << " nZ(w)=" << cPPb.nZw
              << " sumW=" << cPPb.nEvtW << std::endl;
    std::cout << "pp:  nZ(uw)=" << cPP.nZuw << " nZ(w)=" << cPP.nZw
              << " sumW=" << cPP.nEvtW << std::endl;

    // Clone for the two normalization variants
    for (auto* src : {&hPPb, &hPP}) {
        auto* dst = (src == &hPPb) ? &hPPb_uw : &hPP_uw;
        dst->hZPt->Add(src->hZPt);
        dst->hZEta->Add(src->hZEta);
        dst->hTrkPt->Add(src->hTrkPt);
        dst->hTrkEta->Add(src->hTrkEta);
    }

    // Normalization A: 1/NZ(weighted) — divide by sum of EventWeight for Z events
    auto normalizeByW = [](Histos& h, double nZw) {
        double scale = 1.0 / nZw;
        h.hZPt->Scale(scale); h.hZEta->Scale(scale);
        h.hTrkPt->Scale(scale); h.hTrkEta->Scale(scale);
    };
    normalizeByW(hPPb, cPPb.nZw);
    normalizeByW(hPP, cPP.nZw);

    // Normalization B: 1/NZ(unweighted) — divide by count of Z events
    normalizeByW(hPPb_uw, cPPb.nZuw);
    normalizeByW(hPP_uw, cPP.nZuw);

    auto style = [](Histos& h, int color, int marker) {
        for (auto* hist : {h.hZPt, h.hZEta, h.hTrkPt, h.hTrkEta}) {
            hist->SetLineColor(color);
            hist->SetMarkerColor(color);
            hist->SetMarkerStyle(marker);
            hist->SetMarkerSize(0.6);
            hist->SetLineWidth(2);
        }
    };
    style(hPPb, kRed, 20);
    style(hPP, kBlue, 24);
    style(hPPb_uw, kRed, 20);
    style(hPP_uw, kBlue, 24);

    // Plot A: 1/NZ(weighted) normalization
    {
        TCanvas c("c_w", "1/NZ(weighted)", 1400, 1100);
        c.Divide(2, 2);
        drawWithRatio(c, 1, hPPb.hZPt, hPP.hZPt,
                      "Z p_{T} [GeV]", true, true, "z_pt_wNorm", outDir);
        drawWithRatio(c, 2, hPPb.hZEta, hPP.hZEta,
                      "Z #eta", false, false, "z_eta_wNorm", outDir);
        drawWithRatio(c, 3, hPPb.hTrkPt, hPP.hTrkPt,
                      "Track p_{T} [GeV]", true, true, "track_pt_wNorm", outDir);
        drawWithRatio(c, 4, hPPb.hTrkEta, hPP.hTrkEta,
                      "Track #eta", false, false, "track_eta_wNorm", outDir);
        c.SaveAs((outDir + "/validation_pPb_wNorm.pdf").c_str());
        c.SaveAs((outDir + "/validation_pPb_wNorm.png").c_str());
    }

    // Plot B: 1/NZ(unweighted) normalization
    {
        TCanvas c("c_uw", "1/NZ(unweighted)", 1400, 1100);
        c.Divide(2, 2);
        drawWithRatio(c, 1, hPPb_uw.hZPt, hPP_uw.hZPt,
                      "Z p_{T} [GeV]", true, true, "z_pt_uwNorm", outDir);
        drawWithRatio(c, 2, hPPb_uw.hZEta, hPP_uw.hZEta,
                      "Z #eta", false, false, "z_eta_uwNorm", outDir);
        drawWithRatio(c, 3, hPPb_uw.hTrkPt, hPP_uw.hTrkPt,
                      "Track p_{T} [GeV]", true, true, "track_pt_uwNorm", outDir);
        drawWithRatio(c, 4, hPPb_uw.hTrkEta, hPP_uw.hTrkEta,
                      "Track #eta", false, false, "track_eta_uwNorm", outDir);
        c.SaveAs((outDir + "/validation_pPb_uwNorm.pdf").c_str());
        c.SaveAs((outDir + "/validation_pPb_uwNorm.png").c_str());
    }

    std::cout << "Plots saved to " << outDir << std::endl;
    return 0;
}
