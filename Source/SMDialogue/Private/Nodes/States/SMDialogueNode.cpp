// Copyright Recursoft LLC 2019-2020. All Rights Reserved.
#pragma once

#include "SMDialogueNode.h"
#include "SMState.h"
#include "SMTransition.h"
#include "SMDialogueChoiceNode.h"


USMDialogueNode_Base::USMDialogueNode_Base() : Super()
{
#if WITH_EDITORONLY_DATA
	NodeDescription.Name = "Dialogue Node";
	NodeDescription.Category = FText::FromString("Dialogue");
	DialogueBody.WidgetInfo.DefaultText = FText::FromString("Dialogue");
	DialogueBody.WidgetInfo.bConsiderForDefaultWidget = true;
	DialogueBody.WidgetInfo.DisplayOrder = 1;
	bDisplayNameWidget = false;
	bUseCustomColors = true;
	NodeColor = FLinearColor(0.1f, 0.62f, 1.f, 0.7f);
	NodeEndStateColor = FLinearColor(1.f, 1.f, 1.f, 0.7f);
#endif
}

const FText& USMDialogueNode_Base::GetDialogueText() const
{
	const_cast<USMDialogueNode_Base*>(this)->DialogueBody.Execute();
	return DialogueBody.Result;
}

void USMDialogueNode_Base::SetPreviousDialogueSpeaker(UObject* Speaker)
{
	PreviousDialogueSpeaker = Speaker;
}

UObject* USMDialogueNode_Base::GetDialogueSpeaker_Implementation() const
{
	const_cast<USMDialogueNode_Base*>(this)->EvaluateGraphProperties();
	return PreviousDialogueSpeaker;
}


USMDialogueNode::USMDialogueNode() : Super()
{
	bUsePreviousSpeakerIfNoneSet = true;
}

void USMDialogueNode::GetAllDialogueNodes(TArray<USMDialogueNode*>& OutDialogueNodes) const
{
	TArray<USMNodeInstance*> OutNodes;
	GetAllNodesOfType(OutNodes, USMDialogueNode::StaticClass(), true);

	for(USMNodeInstance* OutNode : OutNodes)
	{
		if(USMDialogueNode* Node = Cast<USMDialogueNode>(OutNode))
		{
			OutDialogueNodes.Add(Node);
		}
	}
}

void USMDialogueNode::GetAllConnectedDialogueNodes(TArray<USMDialogueNode*>& OutDialogueNodes,
	bool bBreakOnChoices) const
{
	TArray<UClass*> RequiredTypes;
	RequiredTypes.Add(USMDialogueNode::StaticClass());

	if (!bBreakOnChoices)
	{
		RequiredTypes.Add(USMDialogueChoiceNode::StaticClass());
	}

	TArray<USMNodeInstance*> OutNodes;
	GetAllNodesOfType(OutNodes, USMDialogueNode::StaticClass(), true, RequiredTypes);
	for (USMNodeInstance* OutNode : OutNodes)
	{
		if (USMDialogueNode* Node = Cast<USMDialogueNode>(OutNode))
		{
			OutDialogueNodes.Add(Node);
		}
	}
}

void USMDialogueNode::GetAllDialogueSpeakers(TArray<UObject*>& Speakers, bool bConnectedOnly,
                                             bool bBreakOnChoices) const
{
	TArray<USMDialogueNode*> DialogueNodes;
	if(bConnectedOnly)
	{
		GetAllConnectedDialogueNodes(DialogueNodes, bBreakOnChoices);
	}
	else
	{
		GetAllDialogueNodes(DialogueNodes);
	}
	
	for(USMDialogueNode* Node : DialogueNodes)
	{
		if (UObject* DialogueSpeaker = Node->GetDialogueSpeaker())
		{
			Speakers.AddUnique(DialogueSpeaker);
		}
	}
}

void USMDialogueNode::OnStateBegin_Implementation()
{
}

void USMDialogueNode::OnStateUpdate_Implementation(float DeltaSeconds)
{
}

void USMDialogueNode::OnStateEnd_Implementation()
{
}

UObject* USMDialogueNode::GetDialogueSpeaker_Implementation() const
{
	Super::GetDialogueSpeaker_Implementation();
	return (Speaker || !bUsePreviousSpeakerIfNoneSet) ? Speaker : PreviousDialogueSpeaker;
}

void USMDialogueNode::GetAvailableChoices(TArray<USMDialogueChoiceNode*>& Choices)
{
	if (const FSMState_Base* Node = (FSMState_Base*)GetOwningNode())
	{
		for (FSMTransition* Transition : Node->GetOutgoingTransitions())
		{
			if (USMDialogueChoiceNode* Choice = Cast<USMDialogueChoiceNode>(Transition->GetToState()->GetNodeInstance()))
			{
				const bool bCanEval = Transition->bCanEvaluate;
				Transition->bCanEvaluate = true;
				if (Transition->DoesTransitionPass())
				{
					Choice->SetCurrentDialogueOwner(this);
					Choices.Add(Choice);
				}
				Transition->bCanEvaluate = bCanEval;
			}
		}
	}
}

void USMDialogueNode::SelectChoice(USMDialogueChoiceNode* Choice)
{
	SwitchToLinkedState(Choice, false);
}
