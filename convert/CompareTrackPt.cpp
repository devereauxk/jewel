#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
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
#include "TColor.h"

TH1D* fillTrackPt(TTree* tree, const char* name) {
    const int nBins = 25;
    double bins[nBins + 1];
    for (int i = 0; i <= nBins; i++)
        bins[i] = 0.5 * std::pow(50.0 / 0.5, (double)i / nBins);

    TH1D* h = new TH1D(name, "", nBins, bins);
    h->Sumw2();

    float EventWeight;
    std::vector<float>* trackPt = nullptr;
    std::vector<float>* trackEta = nullptr;
    std::vector<float>* trackWeight = nullptr;
    std::vector<int>* trackCharge = nullptr;

    tree->SetBranchAddress("EventWeight", &EventWeight);
    tree->SetBranchAddress("trackPt", &trackPt);
    tree->SetBranchAddress("trackEta", &trackEta);
    tree->SetBranchAddress("trackWeight", &trackWeight);
    tree->SetBranchAddress("trackCharge", &trackCharge);

    Long64_t n = tree->GetEntries();
    for (Long64_t i = 0; i < n; i++) {
        tree->GetEntry(i);
        if (!trackPt || !trackWeight || !trackCharge) continue;
        for (size_t t = 0; t < trackPt->size(); t++) {
            if (trackWeight->at(t) < 0.5) continue;
            if (trackCharge->at(t) == 999) continue;
            if (std::abs(trackEta->at(t)) > 2.4) continue;
            if (trackPt->at(t) < 0.5) continue;
            h->Fill(trackPt->at(t), EventWeight);
        }
    }

    double integral = h->Integral("width");
    if (integral > 0) h->Scale(1.0 / integral);
    return h;
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

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0]
                  << " baseline240.root baseline220.root outputDir/ variantB.root [variantC.root ...]"
                  << std::endl;
        return 1;
    }

    std::string file240 = argv[1];
    std::string file220 = argv[2];
    std::string outDir = argv[3];
    gSystem->mkdir(outDir.c_str(), true);

    int nVariants = argc - 4;
    std::vector<std::string> variantFiles;
    for (int i = 4; i < argc; i++)
        variantFiles.push_back(argv[i]);

    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);

    auto load = [](const std::string& path, const char* hname) -> TH1D* {
        TFile* f = TFile::Open(path.c_str());
        if (!f || f->IsZombie()) {
            std::cerr << "Cannot open " << path << std::endl;
            return nullptr;
        }
        TTree* t = (TTree*)f->Get("Tree");
        std::cout << hname << " (" << path << "): " << t->GetEntries() << " events" << std::endl;
        return fillTrackPt(t, hname);
    };

    TH1D* h240 = load(file240, "h240_baseline");
    TH1D* h220 = load(file220, "h220_baseline");

    std::vector<TH1D*> hVariants;
    const char* varLabels[] = {
        "B: PARJ(81)=0.29",
        "C: PARJ(82)=1.0",
        "D: MSTJ(22)=1",
        "E: MSTP(125)=1",
        "F: All reverted"
    };
    int varColors[] = {kGreen+2, kMagenta+1, kCyan+2, kOrange+1, kViolet+1};

    for (int i = 0; i < nVariants; i++) {
        char hname[64];
        snprintf(hname, sizeof(hname), "hVar_%d", i);
        TH1D* h = load(variantFiles[i], hname);
        hVariants.push_back(h);
    }

    // --- Panel 1: All variants overlaid ---
    TCanvas c1("c1", "Track pT comparison", 1200, 900);
    TPad* pMain = new TPad("main", "", 0, 0.35, 1, 1);
    pMain->SetBottomMargin(0.02);
    pMain->SetLeftMargin(0.12);
    pMain->SetRightMargin(0.04);
    pMain->SetLogx();
    pMain->SetLogy();
    pMain->Draw();
    pMain->cd();

    h220->SetLineColor(kBlue);
    h220->SetMarkerColor(kBlue);
    h220->SetMarkerStyle(24);
    h220->SetMarkerSize(0.6);
    h220->SetLineWidth(2);

    h240->SetLineColor(kRed);
    h240->SetMarkerColor(kRed);
    h240->SetMarkerStyle(20);
    h240->SetMarkerSize(0.6);
    h240->SetLineWidth(2);

    double maxY = std::max(h240->GetMaximum(), h220->GetMaximum()) * 2.0;
    double minY = maxY * 1e-6;
    h220->GetYaxis()->SetRangeUser(minY, maxY);
    h220->GetXaxis()->SetLabelSize(0);
    h220->GetXaxis()->SetTitleSize(0);
    h220->GetYaxis()->SetTitle("1/N dN/dp_{T} [GeV^{-1}]");
    h220->GetYaxis()->SetTitleSize(0.05);
    h220->GetYaxis()->SetTitleOffset(0.9);
    h220->Draw("E");
    h240->Draw("E SAME");

    for (int i = 0; i < nVariants; i++) {
        hVariants[i]->SetLineColor(varColors[i]);
        hVariants[i]->SetMarkerColor(varColors[i]);
        hVariants[i]->SetMarkerStyle(21 + i);
        hVariants[i]->SetMarkerSize(0.5);
        hVariants[i]->SetLineWidth(2);
        hVariants[i]->Draw("E SAME");
    }

    TLegend* leg = new TLegend(0.40, 0.40, 0.94, 0.92);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.035);
    leg->AddEntry(h240, "2.4.0 baseline (stock)", "lep");
    leg->AddEntry(h220, "2.2.0 baseline (stock)", "lep");
    for (int i = 0; i < nVariants && i < 5; i++)
        leg->AddEntry(hVariants[i], varLabels[i], "lep");
    leg->Draw();

    // Ratio pad: all ratios to 2.2.0
    c1.cd();
    TPad* pRat = new TPad("ratio", "", 0, 0, 1, 0.35);
    pRat->SetTopMargin(0.02);
    pRat->SetBottomMargin(0.30);
    pRat->SetLeftMargin(0.12);
    pRat->SetRightMargin(0.04);
    pRat->SetLogx();
    pRat->Draw();
    pRat->cd();

    TH1D* hRat240 = (TH1D*)h240->Clone("rat240");
    hRat240->Divide(h220);
    hRat240->GetYaxis()->SetRangeUser(0.5, 1.5);
    hRat240->GetYaxis()->SetTitle("Ratio to 2.2.0");
    hRat240->GetYaxis()->SetTitleSize(0.09);
    hRat240->GetYaxis()->SetTitleOffset(0.45);
    hRat240->GetYaxis()->SetLabelSize(0.07);
    hRat240->GetYaxis()->SetNdivisions(505);
    hRat240->GetXaxis()->SetTitle("Track p_{T} [GeV]");
    hRat240->GetXaxis()->SetTitleSize(0.10);
    hRat240->GetXaxis()->SetTitleOffset(1.0);
    hRat240->GetXaxis()->SetLabelSize(0.07);
    hRat240->Draw("E");

    std::vector<TH1D*> hRatVariants;
    for (int i = 0; i < nVariants; i++) {
        TH1D* r = (TH1D*)hVariants[i]->Clone(Form("ratVar_%d", i));
        r->Divide(h220);
        r->Draw("E SAME");
        hRatVariants.push_back(r);
    }

    TLine* line = new TLine(hRat240->GetXaxis()->GetXmin(), 1.0,
                            hRat240->GetXaxis()->GetXmax(), 1.0);
    line->SetLineStyle(2);
    line->SetLineColor(kGray + 2);
    line->Draw();

    c1.SaveAs((outDir + "/track_pt_variants.pdf").c_str());
    c1.SaveAs((outDir + "/track_pt_variants.png").c_str());

    // --- Print chi2 summary ---
    std::cout << "\n=== Chi2/ndf vs 2.2.0 baseline ===" << std::endl;
    int ndf;
    double chi2 = calcChi2(h240, h220, ndf);
    std::cout << "  2.4.0 baseline: chi2/ndf = " << chi2 << "/" << ndf
              << " = " << (ndf > 0 ? chi2 / ndf : 0) << std::endl;
    for (int i = 0; i < nVariants && i < 5; i++) {
        chi2 = calcChi2(hVariants[i], h220, ndf);
        std::cout << "  " << varLabels[i] << ": chi2/ndf = " << chi2 << "/" << ndf
                  << " = " << (ndf > 0 ? chi2 / ndf : 0) << std::endl;
    }

    std::cout << "\n=== Chi2/ndf vs 2.4.0 baseline ===" << std::endl;
    for (int i = 0; i < nVariants && i < 5; i++) {
        chi2 = calcChi2(hVariants[i], h240, ndf);
        std::cout << "  " << varLabels[i] << ": chi2/ndf = " << chi2 << "/" << ndf
                  << " = " << (ndf > 0 ? chi2 / ndf : 0) << std::endl;
    }

    // --- Individual variant panels ---
    for (int i = 0; i < nVariants && i < 5; i++) {
        TCanvas cv(Form("cv_%d", i), varLabels[i], 700, 600);
        TPad* pm = new TPad("pm", "", 0, 0.3, 1, 1);
        pm->SetBottomMargin(0.02);
        pm->SetLeftMargin(0.14);
        pm->SetRightMargin(0.04);
        pm->SetLogx();
        pm->SetLogy();
        pm->Draw();
        pm->cd();

        TH1D* hv = (TH1D*)hVariants[i]->Clone(Form("hv_%d", i));
        TH1D* h2c = (TH1D*)h220->Clone(Form("h220c_%d", i));
        TH1D* h4c = (TH1D*)h240->Clone(Form("h240c_%d", i));

        h2c->GetYaxis()->SetRangeUser(minY, maxY);
        h2c->GetYaxis()->SetTitle("1/N dN/dp_{T}");
        h2c->GetYaxis()->SetTitleSize(0.06);
        h2c->GetXaxis()->SetLabelSize(0);
        h2c->Draw("E");
        h4c->Draw("E SAME");
        hv->Draw("E SAME");

        TLegend* leg2 = new TLegend(0.40, 0.60, 0.94, 0.92);
        leg2->SetBorderSize(0);
        leg2->SetFillStyle(0);
        leg2->SetTextSize(0.04);
        leg2->AddEntry(h4c, "2.4.0 baseline", "lep");
        leg2->AddEntry(h2c, "2.2.0 baseline", "lep");
        leg2->AddEntry(hv, varLabels[i], "lep");
        leg2->Draw();

        int ndf1, ndf2;
        double c1v = calcChi2(hv, h220, ndf1);
        double c2v = calcChi2(h4c, h220, ndf2);
        TLatex tex;
        tex.SetNDC();
        tex.SetTextSize(0.035);
        tex.SetTextColor(varColors[i]);
        tex.DrawLatex(0.16, 0.88, Form("variant vs 2.2.0: #chi^{2}/ndf = %.1f/%d = %.2f",
                      c1v, ndf1, ndf1 > 0 ? c1v / ndf1 : 0));
        tex.SetTextColor(kRed);
        tex.DrawLatex(0.16, 0.82, Form("2.4.0 vs 2.2.0:   #chi^{2}/ndf = %.1f/%d = %.2f",
                      c2v, ndf2, ndf2 > 0 ? c2v / ndf2 : 0));

        cv.cd();
        TPad* pr = new TPad("pr", "", 0, 0, 1, 0.3);
        pr->SetTopMargin(0.02);
        pr->SetBottomMargin(0.35);
        pr->SetLeftMargin(0.14);
        pr->SetRightMargin(0.04);
        pr->SetLogx();
        pr->Draw();
        pr->cd();

        TH1D* rv = (TH1D*)hv->Clone(Form("rv_%d", i));
        rv->Divide(h2c);
        TH1D* r4 = (TH1D*)h4c->Clone(Form("r4_%d", i));
        r4->Divide(h2c);

        r4->GetYaxis()->SetRangeUser(0.5, 1.5);
        r4->GetYaxis()->SetTitle("Ratio to 2.2.0");
        r4->GetYaxis()->SetTitleSize(0.12);
        r4->GetYaxis()->SetTitleOffset(0.45);
        r4->GetYaxis()->SetLabelSize(0.09);
        r4->GetXaxis()->SetTitle("Track p_{T} [GeV]");
        r4->GetXaxis()->SetTitleSize(0.12);
        r4->GetXaxis()->SetTitleOffset(1.0);
        r4->GetXaxis()->SetLabelSize(0.09);
        r4->Draw("E");
        rv->Draw("E SAME");

        TLine* ln = new TLine(r4->GetXaxis()->GetXmin(), 1.0,
                              r4->GetXaxis()->GetXmax(), 1.0);
        ln->SetLineStyle(2);
        ln->SetLineColor(kGray + 2);
        ln->Draw();

        char suffix = 'B' + i;
        cv.SaveAs(Form("%s/track_pt_variant_%c.png", outDir.c_str(), suffix));
    }

    std::cout << "\nPlots saved to " << outDir << std::endl;
    return 0;
}
