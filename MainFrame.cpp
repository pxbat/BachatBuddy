#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/listctrl.h>
#include "Expense.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <map>

// Color Palette Constants from https://coolors.co/palette/f8f9fa-e9ecef-dee2e6-ced4da-adb5bd-6c757d-495057-343a40-212529
namespace ColorPalette {
	const wxColour WHITE_SMOKE(248, 249, 250);      // #F8F9FA
	const wxColour LIGHT_GRAY(233, 236, 239);       // #E9ECEF
	const wxColour PLATINUM(222, 226, 230);         // #DEE2E6
	const wxColour FRENCH_GRAY(206, 212, 218);      // #CED4DA
	const wxColour CADET_GRAY(173, 181, 189);       // #ADB5BD
	const wxColour SLATE_GRAY(108, 117, 125);       // #6C757D
	const wxColour DARK_SLATE_GRAY(73, 80, 87);     // #495057
	const wxColour GUNMETAL(52, 58, 64);            // #343A40
	const wxColour RICH_BLACK(33, 37, 41);          // #212529
}

// Helper to extract all expenses from the list control
std::vector<Expense> GetExpensesFromListCtrl(wxListCtrl* listCtrl) {
	std::vector<Expense> expenses;
	for (int i = 0; i < listCtrl->GetItemCount(); ++i) {
		Expense exp;
		exp.description = listCtrl->GetItemText(i, 0);
		exp.category = listCtrl->GetItemText(i, 1);
		exp.amount = listCtrl->GetItemText(i, 2);
		exp.date = listCtrl->GetItemText(i, 3);
		expenses.push_back(exp);
	}
	return expenses;
}

// Helper to reload expenses into the list control
void LoadExpensesToListCtrl(wxListCtrl* listCtrl, const std::vector<Expense>& expenses) {
	listCtrl->DeleteAllItems();
	for (const auto& exp : expenses) {
		long idx = listCtrl->InsertItem(listCtrl->GetItemCount(), exp.description);
		listCtrl->SetItem(idx, 1, exp.category);
		listCtrl->SetItem(idx, 2, exp.amount);
		listCtrl->SetItem(idx, 3, exp.date);
	}
}


MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
	// Set minimum window size
	SetMinSize(wxSize(800, 600));
	wxIcon appIcon;
	if (appIcon.LoadFile("resources/logo.ico", wxBITMAP_TYPE_ICO))
	{
		SetIcon(appIcon);
	}
	else
	{
		wxLogWarning("Could not load icon file!");
	}


	wxFont appFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Brass Mono");

	// Set font for the entire frame and children
	SetFont(appFont);
	CreateControls();
	BindEvents();
	AddSavedExpense();


	// Enable window resizing
	Centre();
}

void MainFrame::CreateControls() {
	wxFont headLineFont(wxFontInfo(wxSize(0, 30)).Bold().FaceName("Brass Mono"));
	wxFont bodyFont(wxFontInfo(wxSize(0, 16)).FaceName("Brass Mono"));

	panel = new wxPanel(this);
	panel->SetFont(bodyFont);

	// Create main sizer for the entire panel
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	// Header
	headLineText = new wxStaticText(panel, wxID_ANY, "BachatBuddy", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	headLineText->SetBackgroundColour(ColorPalette::CADET_GRAY);
	headLineText->SetForegroundColour(ColorPalette::WHITE_SMOKE);
	headLineText->SetFont(headLineFont);
	mainSizer->Add(headLineText, 0, wxEXPAND | wxALL, 10);

	/* Input section*/
	inputBox = new wxStaticBox(panel, wxID_ANY, "Add New Expense");
	wxStaticBoxSizer* inputSizer = new wxStaticBoxSizer(inputBox, wxVERTICAL);

	// First row: Description and Category
	wxBoxSizer* row1Sizer = new wxBoxSizer(wxHORIZONTAL);

	// Description
	wxBoxSizer* descSizer = new wxBoxSizer(wxVERTICAL);
	descText = new wxStaticText(panel, wxID_ANY, "Description");
	descInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	descSizer->Add(descText, 0, wxBOTTOM, 5);
	descSizer->Add(descInput, 1, wxEXPAND);
	row1Sizer->Add(descSizer, 1, wxRIGHT, 10);

	// Category
	wxBoxSizer* catSizer = new wxBoxSizer(wxVERTICAL);
	catText = new wxStaticText(panel, wxID_ANY, "Category");
	catInput = new wxComboBox(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN);
	catSizer->Add(catText, 0, wxBOTTOM, 5);
	catSizer->Add(catInput, 1, wxEXPAND);
	row1Sizer->Add(catSizer, 1, wxLEFT, 10);

	inputSizer->Add(row1Sizer, 0, wxEXPAND | wxALL, 5);

	// Second row: Amount, Date, and Add button
	wxBoxSizer* row2Sizer = new wxBoxSizer(wxHORIZONTAL);

	// Amount
	wxBoxSizer* amountSizer = new wxBoxSizer(wxVERTICAL);
	amountText = new wxStaticText(panel, wxID_ANY, "Amount");
	amountInput = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	amountSizer->Add(amountText, 0, wxBOTTOM, 5);
	amountSizer->Add(amountInput, 1, wxEXPAND);
	row2Sizer->Add(amountSizer, 1, wxRIGHT, 10);

	// Date
	wxBoxSizer* dateSizer = new wxBoxSizer(wxVERTICAL);
	dateText = new wxStaticText(panel, wxID_ANY, "Date");
	dateInput = new wxDatePickerCtrl(panel, wxID_ANY, wxDefaultDateTime);
	dateSizer->Add(dateText, 0, wxBOTTOM, 5);
	dateSizer->Add(dateInput, 1, wxEXPAND);
	row2Sizer->Add(dateSizer, 1, wxLEFT | wxRIGHT, 10);

	// Add button
	addButton = new wxButton(panel, wxID_ANY, "Add");
	addButton->SetMinSize(wxSize(100, -1));
	row2Sizer->Add(addButton, 0, wxLEFT | wxALIGN_BOTTOM, 10);

	inputSizer->Add(row2Sizer, 0, wxEXPAND | wxALL, 5);

	mainSizer->Add(inputSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

	// List control for expenses
	listCtrl = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
	listCtrl->Bind(wxEVT_SIZE, &MainFrame::OnListCtrlResize, this);

	listCtrl->InsertColumn(0, "Description", wxLIST_FORMAT_CENTER, 250);
	listCtrl->InsertColumn(1, "Category", wxLIST_FORMAT_CENTER, 170);
	listCtrl->InsertColumn(2, "Amount", wxLIST_FORMAT_CENTER, 170);
	listCtrl->InsertColumn(3, "Date", wxLIST_FORMAT_CENTER, 170);

	mainSizer->Add(listCtrl, 1, wxEXPAND |wxALL, 10);

	// Bottom button row
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	settingsButton = new wxButton(panel, wxID_ANY, "Toggle", wxDefaultPosition, wxSize(100, 40));
	settingsButton->SetFont(wxFontInfo(9).Bold().FaceName("Brass Mono"));
	settingsButton->SetBackgroundColour(ColorPalette::SLATE_GRAY);
	settingsButton->SetForegroundColour(ColorPalette::WHITE_SMOKE);
	settingsButton->SetToolTip("Toggle Theme");



	clearButton = new wxButton(panel, wxID_ANY, "Clear");
	clearButton->SetMinSize(wxSize(100, -1));

	buttonSizer->Add(settingsButton, 0, wxRIGHT, 10);
	buttonSizer->AddStretchSpacer(1);

	buttonSizer->Add(clearButton, 0);

	mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 10);

	panel->SetSizer(mainSizer);

	// Apply initial light theme
	ApplyDarkTheme();
}

void MainFrame::OnListCtrlResize(wxSizeEvent& event)
{
	AdjustColumns();
	event.Skip();
}

void MainFrame::AdjustColumns()
{
	int totalW = listCtrl->GetClientSize().GetWidth();

	const int minDesc = 150, minCat = 100, minAmt = 100, minDate = 100;
	const double pDesc = 0.35, pCat = 0.22, pAmt = 0.22;
	int wDesc = wxMax((int)(totalW * pDesc), minDesc);
	int wCat = wxMax((int)(totalW * pCat), minCat);
	int wAmt = wxMax((int)(totalW * pAmt), minAmt);
	int wDate = totalW - (wDesc + wCat + wAmt);
	wDate = wxMax(wDate, minDate);

	int sumMin = minDesc + minCat + minAmt + minDate;
	if (sumMin > totalW)
	{
		int eq = totalW / 4;
		wDesc = wCat = wAmt = wDate = eq;
		wDate += totalW - (eq * 4);
	}
	else
	{
		int used = wDesc + wCat + wAmt + wDate;
		wDate += totalW - used;
	}

	listCtrl->SetColumnWidth(0, wDesc);
	listCtrl->SetColumnWidth(1, wCat);
	listCtrl->SetColumnWidth(2, wAmt);
	listCtrl->SetColumnWidth(3, wDate);
}

void MainFrame::OnResize(wxSizeEvent& event)
{
	event.Skip();  // Allow default handling too
}

void MainFrame::BindEvents() {
	// Binding event handlers to controls
	addButton->Bind(wxEVT_BUTTON, &MainFrame::OnAddButtonClicked, this);
	clearButton->Bind(wxEVT_BUTTON, &MainFrame::OnClearButtonClicked, this);
	descInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnInputEnter, this);
	catInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnInputEnter, this);
	amountInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnInputEnter, this);
	dateInput->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnInputEnter, this);
	listCtrl->Bind(wxEVT_KEY_DOWN, &MainFrame::OnKeyDown, this);
	this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnWindowClosed, this);
	listCtrl->Bind(wxEVT_LIST_COL_CLICK, &MainFrame::OnListColClick, this);
	settingsButton->Bind(wxEVT_BUTTON, &MainFrame::OnSettingsButtonClicked, this);
}

void MainFrame::AddExpenseFromInput() {
	wxString desc = descInput->GetValue();
	wxString cat = catInput->GetValue();
	// Add new category to the combo box if not already present
	if (!cat.IsEmpty() && catInput->FindString(cat) == wxNOT_FOUND) {
		catInput->Append(cat);
		categoryList.push_back(cat);
	}
	wxString amount = amountInput->GetValue();
	wxString date = dateInput->GetValue().FormatISODate();

	// Error handling for when all of the fields are not filled
	if (desc.IsEmpty() || cat.IsEmpty() || amount.IsEmpty() || date.IsEmpty()) {
		wxMessageBox("Please fill in all the fields!");
		return;
	}

	// Adding each field values to the list
	long index = listCtrl->InsertItem(listCtrl->GetItemCount(), desc);
	listCtrl->SetItem(index, 1, cat);
	listCtrl->SetItem(index, 2, amount);
	listCtrl->SetItem(index, 3, date);

	// Clearing the input field after the values of the input fields have been listed
	descInput->Clear();
	catInput->SetValue(""); // Only clear the text, not the items!
	amountInput->Clear();
}

// Deleting selected expense detail when delete key is pressed
void MainFrame::OnKeyDown(wxKeyEvent& evt) {
	switch (evt.GetKeyCode()) {
	case WXK_DELETE:
		DeleteExpense();
		break;
	}
}

void MainFrame::DeleteExpense() {
	long index = listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	if (index == -1) {
		wxMessageBox("No task is selected!");
		return;
	}

	listCtrl->DeleteItem(index);
}

// Event handling when add button is pressed
void MainFrame::OnAddButtonClicked(wxCommandEvent& evt) {
	AddExpenseFromInput();
}

// Event handling when enter key is pressed after filling the fields
void MainFrame::OnInputEnter(wxCommandEvent& evt) {
	AddExpenseFromInput();
}

void MainFrame::OnClearButtonClicked(wxCommandEvent& evt) {
	// Error handling for when the Clear button is pressed when there are no items in the list
	if (listCtrl->GetItemCount() == 0) {
		wxMessageBox("There are no expenses!");
		return;
	}

	// Dialog box to confirm that the user wants to clear the list
	wxMessageDialog dialog(this, "Are you sure you want to clear all expenses?", "Clear", wxYES_NO | wxCANCEL);

	// Storing enum ID of the dialog input
	int result = dialog.ShowModal();

	// If the enum ID is matching with the enum ID of the yes button, then clear the input
	if (result == wxID_YES) {
		listCtrl->DeleteAllItems();
	}
}

void MainFrame::OnMouseEnter(wxMouseEvent& event) {
	wxButton* btn = dynamic_cast<wxButton*>(event.GetEventObject());
	if (btn) btn->SetBackgroundColour(ColorPalette::SLATE_GRAY);  // hover color
	event.Skip();
}

void MainFrame::OnMouseLeave(wxMouseEvent& event) {
	wxButton* btn = dynamic_cast<wxButton*>(event.GetEventObject());
	if (btn) btn->SetBackgroundColour(ColorPalette::DARK_SLATE_GRAY);  // default button color
	event.Skip();
}


// Event Handling when the window is closed i.e., saving the list items in a text file
void MainFrame::OnWindowClosed(wxCloseEvent& evt) {
	// Declaring a vector of Expense structures
	std::vector<Expense> expenses;

	// Iterating through the list
	for (int i = 0; i < listCtrl->GetItemCount(); i++) {
		Expense expense;           // Declaring a Expense structure
		expense.description = listCtrl->GetItemText(i, 0); // Adding values from the list's column to the corresponding structure members
		expense.category = listCtrl->GetItemText(i, 1);
		expense.amount = listCtrl->GetItemText(i, 2);
		expense.date = listCtrl->GetItemText(i, 3);
		expenses.push_back(expense); // Adding the structure to the vector of structures
	}
	AddExpenseToFile(expenses, "expense.txt");  // Storing the vector in the text file
	evt.Skip();  // skipping event to prevent the window from not closing
}

// Adding saved expenses to the list from the text file to the list after the app has been re-opened
void MainFrame::AddSavedExpense() {
	catInput->Clear();
	categoryList.clear();
	std::vector<Expense> expenses = LoadExpenseFromFile("expense.txt");

	for (const Expense& expense : expenses) {
		int index = listCtrl->InsertItem(listCtrl->GetItemCount(), expense.description);

		listCtrl->SetItem(index, 1, expense.category);
		listCtrl->SetItem(index, 2, expense.amount);
		listCtrl->SetItem(index, 3, expense.date);

		// Add category to the combo box if it doesn't already exist
		if (!expense.category.empty() && catInput->FindString(expense.category) == wxNOT_FOUND) {
			catInput->Append(expense.category);
			categoryList.push_back(expense.category);
		}
	}
}

void MainFrame::OnListColClick(wxListEvent& event) {
	int col = event.GetColumn();
	auto expenses = GetExpensesFromListCtrl(listCtrl);

	if (col == 1) { // Category column
		if (categorySortAscending) {
			std::sort(expenses.begin(), expenses.end(), [](const Expense& a, const Expense& b) {
				return a.category < b.category;
				});
		}
		else {
			std::sort(expenses.begin(), expenses.end(), [](const Expense& a, const Expense& b) {
				return a.category > b.category;
				});
		}
		categorySortAscending = !categorySortAscending;
		LoadExpensesToListCtrl(listCtrl, expenses);
	}
	else if (col == 2) { // Amount column
		if (amountSortAscending) {
			std::sort(expenses.begin(), expenses.end(), [](const Expense& a, const Expense& b) {
				try {
					return std::stod(a.amount) < std::stod(b.amount);
				}
				catch (...) {
					return a.amount < b.amount; // fallback to string compare
				}
				});
		}
		else {
			std::sort(expenses.begin(), expenses.end(), [](const Expense& a, const Expense& b) {
				try {
					return std::stod(a.amount) > std::stod(b.amount);
				}
				catch (...) {
					return a.amount > b.amount; // fallback to string compare
				}
				});
		}
		amountSortAscending = !amountSortAscending;
		LoadExpensesToListCtrl(listCtrl, expenses);
	}
	else if (col == 3) { // Date column
		if (dateSortAscending) {
			std::sort(expenses.begin(), expenses.end(), [](const Expense& a, const Expense& b) {
				return a.date < b.date;
				});
		}
		else {
			std::sort(expenses.begin(), expenses.end(), [](const Expense& a, const Expense& b) {
				return a.date > b.date;
				});
		}
		dateSortAscending = !dateSortAscending;
		LoadExpensesToListCtrl(listCtrl, expenses);
	}
}

void MainFrame::OnSettingsButtonClicked(wxCommandEvent& evt) {
	isDarkMode = !isDarkMode;
	if (isDarkMode)
		ApplyDarkTheme();
	else
		ApplyLightTheme();
	panel->Refresh();
}





// Light Theme using grayscale palette
void MainFrame::ApplyLightTheme() {
	// Main panel - lightest shade for background
	panel->SetBackgroundColour(ColorPalette::WHITE_SMOKE);

	//Input sizer text color:
	inputBox->SetForegroundColour(ColorPalette::DARK_SLATE_GRAY);


	// Header - medium accent color
	headLineText->SetBackgroundColour(ColorPalette::CADET_GRAY);
	headLineText->SetForegroundColour(ColorPalette::WHITE_SMOKE);

	// Labels - dark text on light background
	descText->SetForegroundColour(ColorPalette::DARK_SLATE_GRAY);
	catText->SetForegroundColour(ColorPalette::DARK_SLATE_GRAY);
	amountText->SetForegroundColour(ColorPalette::DARK_SLATE_GRAY);
	dateText->SetForegroundColour(ColorPalette::DARK_SLATE_GRAY);

	// Input fields - clean white with dark text
	descInput->SetBackgroundColour(ColorPalette::LIGHT_GRAY);
	descInput->SetForegroundColour(ColorPalette::GUNMETAL);
	catInput->SetBackgroundColour(ColorPalette::LIGHT_GRAY);
	catInput->SetForegroundColour(ColorPalette::GUNMETAL);
	amountInput->SetBackgroundColour(ColorPalette::LIGHT_GRAY);
	amountInput->SetForegroundColour(ColorPalette::GUNMETAL);
	dateInput->SetBackgroundColour(ColorPalette::LIGHT_GRAY);
	dateInput->SetForegroundColour(ColorPalette::GUNMETAL);

	// Buttons - medium gray with good contrast
	addButton->SetBackgroundColour(ColorPalette::FRENCH_GRAY);
	addButton->SetForegroundColour(ColorPalette::DARK_SLATE_GRAY);
	clearButton->SetBackgroundColour(ColorPalette::FRENCH_GRAY);
	clearButton->SetForegroundColour(ColorPalette::DARK_SLATE_GRAY);


	// Settings button - accent color
	settingsButton->SetBackgroundColour(ColorPalette::SLATE_GRAY);
	settingsButton->SetForegroundColour(ColorPalette::WHITE_SMOKE);

	// List control - clean appearance
	listCtrl->SetBackgroundColour(ColorPalette::LIGHT_GRAY);
	listCtrl->SetForegroundColour(ColorPalette::GUNMETAL);

	panel->Refresh();
}

// Dark Theme using grayscale palette
void MainFrame::ApplyDarkTheme() {
	// Main panel - darkest shade for background
	panel->SetBackgroundColour(ColorPalette::RICH_BLACK);

	//Input sizer color:
	inputBox->SetForegroundColour(ColorPalette::WHITE_SMOKE);

	// Header - medium dark with light text
	headLineText->SetBackgroundColour(ColorPalette::GUNMETAL);
	headLineText->SetForegroundColour(ColorPalette::WHITE_SMOKE);

	// Labels - light text on dark background
	descText->SetForegroundColour(ColorPalette::LIGHT_GRAY);
	catText->SetForegroundColour(ColorPalette::LIGHT_GRAY);
	amountText->SetForegroundColour(ColorPalette::LIGHT_GRAY);
	dateText->SetForegroundColour(ColorPalette::LIGHT_GRAY);

	// Input fields - dark background with light text
	descInput->SetBackgroundColour(ColorPalette::DARK_SLATE_GRAY);
	descInput->SetForegroundColour(ColorPalette::WHITE_SMOKE);
	catInput->SetBackgroundColour(ColorPalette::DARK_SLATE_GRAY);
	catInput->SetForegroundColour(ColorPalette::WHITE_SMOKE);
	amountInput->SetBackgroundColour(ColorPalette::DARK_SLATE_GRAY);
	amountInput->SetForegroundColour(ColorPalette::WHITE_SMOKE);
	dateInput->SetBackgroundColour(ColorPalette::DARK_SLATE_GRAY);
	dateInput->SetForegroundColour(ColorPalette::WHITE_SMOKE);

	// Buttons - medium dark with light text
	addButton->SetBackgroundColour(ColorPalette::SLATE_GRAY);
	addButton->SetForegroundColour(ColorPalette::WHITE_SMOKE);
	clearButton->SetBackgroundColour(ColorPalette::SLATE_GRAY);
	clearButton->SetForegroundColour(ColorPalette::WHITE_SMOKE);


	// Settings button - accent in dark theme
	settingsButton->SetBackgroundColour(ColorPalette::CADET_GRAY);
	settingsButton->SetForegroundColour(ColorPalette::RICH_BLACK);

	// List control - dark theme
	listCtrl->SetBackgroundColour(ColorPalette::GUNMETAL);
	listCtrl->SetForegroundColour(ColorPalette::WHITE_SMOKE);

	wxButton* buttons[] = { settingsButton, clearButton, addButton };

	for (wxButton* btn : buttons) {
		btn->SetBackgroundColour(ColorPalette::DARK_SLATE_GRAY);  // default color
		btn->SetForegroundColour(ColorPalette::WHITE_SMOKE);      // text color
		btn->Bind(wxEVT_ENTER_WINDOW, &MainFrame::OnMouseEnter, this);
		btn->Bind(wxEVT_LEAVE_WINDOW, &MainFrame::OnMouseLeave, this);
	}



	panel->Refresh();
}

