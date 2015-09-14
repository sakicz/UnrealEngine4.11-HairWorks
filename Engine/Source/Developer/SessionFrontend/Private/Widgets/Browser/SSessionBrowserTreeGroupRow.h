// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once


#define LOCTEXT_NAMESPACE "SSessionBrowserTreeRow"


/**
 * Implements a row widget for the session browser tree.
 */
class SSessionBrowserTreeGroupRow
	: public STableRow<TSharedPtr<FSessionBrowserGroupTreeItem>>
{
public:

	SLATE_BEGIN_ARGS(SSessionBrowserTreeGroupRow) { }
		SLATE_ATTRIBUTE(FText, HighlightText)
		SLATE_ARGUMENT(TSharedPtr<STableViewBase>, OwnerTableView)
		SLATE_ARGUMENT(TSharedPtr<FSessionBrowserGroupTreeItem>, Item)
	SLATE_END_ARGS()

public:

	/**
	 * Constructs the widget.
	 *
	 * @param InArgs The construction arguments.
	 */
	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		HighlightText = InArgs._HighlightText;
		Item = InArgs._Item;

		ChildSlot
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.0f))
					.BorderImage(this, &SSessionBrowserTreeGroupRow::HandleBorderBackgroundImage)
					.Padding(3.0f)
					.ToolTipText(Item->GetToolTipText())
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(Item->GetGroupName())
									.Font(FEditorStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
									.ShadowOffset(FVector2D(1.0f, 1.0f))
							]

						+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
									.Image(this, &SSessionBrowserTreeGroupRow::HandlePullDownImage)
							]
					]
			];

		STableRow<TSharedPtr<FSessionBrowserGroupTreeItem>>::ConstructInternal(
			STableRow<TSharedPtr<FSessionBrowserGroupTreeItem>>::FArguments()
				.ShowSelection(false)
				.Style(FEditorStyle::Get(), "DetailsView.TreeView.TableRow"),
			InOwnerTableView
		);
	}
	END_SLATE_FUNCTION_BUILD_OPTIMIZATION

protected:

	// SWidget overrides

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			ToggleExpansion();

			return FReply::Handled();
		}
		else
		{
			return FReply::Unhandled();
		}
	}

private:

	/** Callback for getting the background image of the row's border. */
	const FSlateBrush* HandleBorderBackgroundImage() const
	{
		if (IsHovered())
		{
			return IsItemExpanded()
				? FEditorStyle::GetBrush("DetailsView.CategoryTop_Hovered")
				: FEditorStyle::GetBrush("DetailsView.CollapsedCategory_Hovered");
		}
		else
		{
			return IsItemExpanded()
				? FEditorStyle::GetBrush("DetailsView.CategoryTop")
				: FEditorStyle::GetBrush("DetailsView.CollapsedCategory");
		}
	}

	/** Gets the image for the pull-down icon. */
	const FSlateBrush* HandlePullDownImage() const
	{
		if (IsHovered())
		{
			return IsItemExpanded()
				? FEditorStyle::GetBrush("DetailsView.PulldownArrow.Up.Hovered")
				: FEditorStyle::GetBrush("DetailsView.PulldownArrow.Down.Hovered");
		}
		else
		{
			return IsItemExpanded()
				? FEditorStyle::GetBrush("DetailsView.PulldownArrow.Up")
				: FEditorStyle::GetBrush("DetailsView.PulldownArrow.Down");
		}
	}

private:

	/** The highlight string for this row. */
	TAttribute<FText> HighlightText;

	/** A reference to the tree item that is displayed in this row. */
	TSharedPtr<FSessionBrowserGroupTreeItem> Item;
};


#undef LOCTEXT_NAMESPACE