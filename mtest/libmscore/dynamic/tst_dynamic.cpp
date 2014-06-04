//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "libmscore/score.h"
#include "libmscore/dynamic.h"
#include "mtest/testutils.h"

using namespace Ms;

//---------------------------------------------------------
//   TestDynamic
//---------------------------------------------------------

class TestDynamic : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void test1();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestDynamic::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   test1
//    read write test
//---------------------------------------------------------

void TestDynamic::test1()
      {
      Dynamic* dynamic = new Dynamic(score);
      dynamic->setDynamicType(Dynamic::DynamicType(1));

      dynamic->setPlacement(Placement::ABOVE);
      Dynamic* d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::ABOVE);
      delete d;

      dynamic->setPlacement(Placement::BELOW);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::BELOW);
      delete d;

      dynamic->setProperty(P_ID::PLACEMENT, int(Placement::ABOVE));
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::ABOVE);
      delete d;

      dynamic->setProperty(P_ID::PLACEMENT, int(Placement::BELOW));
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->placement(), Placement::BELOW);
      delete d;

      dynamic->setVelocity(23);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 23);
      delete d;

      dynamic->setVelocity(57);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 57);
      delete d;

      dynamic->setProperty(P_ID::VELOCITY, 23);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 23);
      delete d;

      dynamic->setProperty(P_ID::VELOCITY, 57);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->velocity(), 57);
      delete d;

      dynamic->setDynRange(DynamicRange::STAFF);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), DynamicRange::STAFF);
      delete d;

      dynamic->setDynRange(DynamicRange::PART);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), DynamicRange::PART);
      delete d;

      dynamic->setDynRange(DynamicRange::SYSTEM);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), DynamicRange::SYSTEM);
      delete d;

      dynamic->setProperty(P_ID::DYNAMIC_RANGE, int(DynamicRange::STAFF));
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), DynamicRange::STAFF);
      delete d;

      dynamic->setProperty(P_ID::DYNAMIC_RANGE, int(DynamicRange::PART));
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), DynamicRange::PART);
      delete d;

      dynamic->setDynRange(DynamicRange::SYSTEM);
      d = static_cast<Dynamic*>(writeReadElement(dynamic));
      QCOMPARE(d->dynRange(), DynamicRange::SYSTEM);
      delete d;


      }

QTEST_MAIN(TestDynamic)

#include "tst_dynamic.moc"

