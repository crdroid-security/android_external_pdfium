// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_pushbutton.h"

#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_eventmouse.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/ifwl_themeprovider.h"

CFWL_PushButton::CFWL_PushButton(const CFWL_App* app)
    : CFWL_Widget(app, pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr),
      m_bBtnDown(false),
      m_dwTTOStyles(FDE_TTOSTYLE_SingleLine),
      m_iTTOAlign(FDE_TTOALIGNMENT_Center) {
  m_rtClient.Set(0, 0, 0, 0);
  m_rtCaption.Set(0, 0, 0, 0);
}

CFWL_PushButton::~CFWL_PushButton() {}

FWL_Type CFWL_PushButton::GetClassID() const {
  return FWL_Type::PushButton;
}

void CFWL_PushButton::SetStates(uint32_t dwStates) {
  if (dwStates & FWL_WGTSTATE_Disabled) {
    m_pProperties->m_dwStates = FWL_WGTSTATE_Disabled;
    return;
  }
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_PushButton::Update() {
  if (IsLocked())
    return;
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();

  UpdateTextOutStyles();
  m_rtClient = GetClientRect();
  m_rtCaption = m_rtClient;
  FX_FLOAT* fcaption =
      static_cast<FX_FLOAT*>(GetThemeCapacity(CFWL_WidgetCapacity::Margin));
  m_rtCaption.Inflate(-*fcaption, -*fcaption);
}

void CFWL_PushButton::DrawWidget(CFX_Graphics* pGraphics,
                                 const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!m_pProperties->m_pThemeProvider)
    return;

  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_Part::Border, m_pProperties->m_pThemeProvider,
               pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, CFWL_Part::Edge, m_pProperties->m_pThemeProvider,
             pMatrix);
  }
  DrawBkground(pGraphics, m_pProperties->m_pThemeProvider, pMatrix);
}

void CFWL_PushButton::DrawBkground(CFX_Graphics* pGraphics,
                                   IFWL_ThemeProvider* pTheme,
                                   const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = CFWL_Part::Background;
  param.m_dwStates = GetPartStates();
  param.m_pGraphics = pGraphics;
  if (pMatrix)
    param.m_matrix.Concat(*pMatrix);
  param.m_rtPart = m_rtClient;
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)
    param.m_pData = &m_rtCaption;
  pTheme->DrawBackground(&param);
}

uint32_t CFWL_PushButton::GetPartStates() {
  uint32_t dwStates = CFWL_PartState_Normal;
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)
    dwStates |= CFWL_PartState_Focused;
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
    dwStates = CFWL_PartState_Disabled;
  else if (m_pProperties->m_dwStates & FWL_STATE_PSB_Pressed)
    dwStates |= CFWL_PartState_Pressed;
  else if (m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered)
    dwStates |= CFWL_PartState_Hovered;
  return dwStates;
}

void CFWL_PushButton::UpdateTextOutStyles() {
  m_iTTOAlign = FDE_TTOALIGNMENT_TopLeft;
  m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
  if (m_pProperties->m_dwStyleExes & FWL_WGTSTYLE_RTLReading)
    m_dwTTOStyles |= FDE_TTOSTYLE_RTL;
}

void CFWL_PushButton::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;
  if (!IsEnabled())
    return;

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::SetFocus:
      OnFocusChanged(pMessage, true);
      break;
    case CFWL_Message::Type::KillFocus:
      OnFocusChanged(pMessage, false);
      break;
    case CFWL_Message::Type::Mouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case FWL_MouseCommand::Move:
          OnMouseMove(pMsg);
          break;
        case FWL_MouseCommand::Leave:
          OnMouseLeave(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::Key: {
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->m_dwCmd == FWL_KeyCommand::KeyDown)
        OnKeyDown(pKey);
      break;
    }
    default:
      break;
  }
  CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_PushButton::OnDrawWidget(CFX_Graphics* pGraphics,
                                   const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}

void CFWL_PushButton::OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (bSet)
    m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
  else
    m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;

  RepaintRect(m_rtClient);
}

void CFWL_PushButton::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)
    SetFocus(true);

  m_bBtnDown = true;
  m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
  m_pProperties->m_dwStates |= FWL_STATE_PSB_Pressed;
  RepaintRect(m_rtClient);
}

void CFWL_PushButton::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  m_bBtnDown = false;
  if (m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
    m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
  } else {
    m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Hovered;
    m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
  }
  if (m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
    CFWL_Event wmClick(CFWL_Event::Type::Click, this);
    DispatchEvent(&wmClick);
  }
  RepaintRect(m_rtClient);
}

void CFWL_PushButton::OnMouseMove(CFWL_MessageMouse* pMsg) {
  bool bRepaint = false;
  if (m_bBtnDown) {
    if (m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy)) {
      if ((m_pProperties->m_dwStates & FWL_STATE_PSB_Pressed) == 0) {
        m_pProperties->m_dwStates |= FWL_STATE_PSB_Pressed;
        bRepaint = true;
      }
      if (m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered) {
        m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Hovered;
        bRepaint = true;
      }
    } else {
      if (m_pProperties->m_dwStates & FWL_STATE_PSB_Pressed) {
        m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
        bRepaint = true;
      }
      if ((m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered) == 0) {
        m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
        bRepaint = true;
      }
    }
  } else {
    if (!m_rtClient.Contains(pMsg->m_fx, pMsg->m_fy))
      return;
    if ((m_pProperties->m_dwStates & FWL_STATE_PSB_Hovered) == 0) {
      m_pProperties->m_dwStates |= FWL_STATE_PSB_Hovered;
      bRepaint = true;
    }
  }
  if (bRepaint)
    RepaintRect(m_rtClient);
}

void CFWL_PushButton::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  m_bBtnDown = false;
  m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Hovered;
  m_pProperties->m_dwStates &= ~FWL_STATE_PSB_Pressed;
  RepaintRect(m_rtClient);
}

void CFWL_PushButton::OnKeyDown(CFWL_MessageKey* pMsg) {
  if (pMsg->m_dwKeyCode != FWL_VKEY_Return)
    return;

  CFWL_EventMouse wmMouse(this);
  wmMouse.m_dwCmd = FWL_MouseCommand::LeftButtonUp;
  DispatchEvent(&wmMouse);

  CFWL_Event wmClick(CFWL_Event::Type::Click, this);
  DispatchEvent(&wmClick);
}
