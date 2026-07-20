void plot_weight_vs_zpt() {
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);

    auto f1 = TFile::Open("jewel_pp_10k.root");
    auto f2 = TFile::Open("/raid5/data/kdevero/jewel_workspace/jewel_pp-v7.root");
    auto t1 = (TTree*)f1->Get("Tree");
    auto t2 = (TTree*)f2->Get("Tree");

    auto c = new TCanvas("c", "", 1200, 500);
    c->Divide(2, 1);

    c->cd(1);
    gPad->SetLogz();
    gPad->SetLeftMargin(0.14);
    gPad->SetRightMargin(0.16);
    gPad->SetBottomMargin(0.12);
    auto h1 = new TH2D("h1", "", 50, 0, 3, 50, -15, -4);
    h1->GetXaxis()->SetTitle("log_{10}(Z p_{T} [GeV])");
    h1->GetYaxis()->SetTitle("log_{10}(EventWeight)");
    h1->GetXaxis()->SetTitleSize(0.05);
    h1->GetYaxis()->SetTitleSize(0.05);
    h1->GetXaxis()->SetTitleOffset(1.1);
    h1->GetYaxis()->SetTitleOffset(1.3);
    t1->Draw("log10(EventWeight):log10(genZPt)>>h1", "genZPt@.size()>0", "COLZ");
    auto tex = new TLatex();
    tex->SetNDC();
    tex->SetTextSize(0.05);
    tex->DrawLatex(0.16, 0.92, "JEWEL 2.2.0 (this work)");

    c->cd(2);
    gPad->SetLogz();
    gPad->SetLeftMargin(0.14);
    gPad->SetRightMargin(0.16);
    gPad->SetBottomMargin(0.12);
    auto h2 = new TH2D("h2", "", 50, 0, 3, 50, -15, -4);
    h2->GetXaxis()->SetTitle("log_{10}(Z p_{T} [GeV])");
    h2->GetYaxis()->SetTitle("log_{10}(EventWeight)");
    h2->GetXaxis()->SetTitleSize(0.05);
    h2->GetYaxis()->SetTitleSize(0.05);
    h2->GetXaxis()->SetTitleOffset(1.1);
    h2->GetYaxis()->SetTitleOffset(1.3);
    t2->Draw("log10(EventWeight):log10(genZPt)>>h2", "genZPt@.size()>0", "COLZ");
    tex->DrawLatex(0.16, 0.92, "Reference v7");

    c->SaveAs("plots/weight_vs_zpt.png");
}
