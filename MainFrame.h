#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <vector>
#include "Expense.h"

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title);

private:
    // Control declarations
    wxPanel* panel;
    wxStaticText* headLineText;
    wxStaticText* descText;
    wxTextCtrl* descInput;
    wxStaticText* catText;
    wxComboBox* catInput;
    wxStaticText* amountText;
    wxTextCtrl* amountInput;
    wxStaticText* dateText;
    wxDatePickerCtrl* dateInput;
    wxButton* addButton;
    wxListCtrl* listCtrl;
    wxButton* clearButton;
    wxButton* settingsButton;
    wxStaticBox* inputBox;



    wxButton* viewTotalsButton;



    void OnViewTotalsButtonClicked(wxCommandEvent& evt);


    // Member variables
    std::vector<wxString> categoryList;
    bool isDarkMode = false;
    bool categorySortAscending = true;
    bool amountSortAscending = true;
    bool dateSortAscending = true;

    // Methods
    void CreateControls();
    void BindEvents();
    void AddExpenseFromInput();
    void DeleteExpense();
    void EnableDarkMode();
    void EnableLightMode();

    // Event handlers
    void OnAddButtonClicked(wxCommandEvent& evt);
    void OnInputEnter(wxCommandEvent& evt);
    void OnClearButtonClicked(wxCommandEvent& evt);
    void OnKeyDown(wxKeyEvent& evt);
    void OnWindowClosed(wxCloseEvent& evt);
    void OnListColClick(wxListEvent& event);
    void OnSettingsButtonClicked(wxCommandEvent& evt);
    void OnPlotButtonClicked(wxCommandEvent& evt);
    void OnResize(wxSizeEvent& event);  // Add this line
    void ApplyLightTheme();
    void ApplyDarkTheme();
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnListCtrlResize(wxSizeEvent& event);
    void AdjustColumns();

    // File operations
    void AddSavedExpense();
};