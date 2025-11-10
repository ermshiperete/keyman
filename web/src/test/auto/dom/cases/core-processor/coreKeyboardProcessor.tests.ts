/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */

import { assert } from 'chai';
import sinon from 'sinon';
import { KM_Core, km_core_context, km_core_keyboard, km_core_state, KM_CORE_CT, KM_CORE_STATUS, unitTestEndpoints } from 'keyman/engine/core-adapter';
import { coreurl, loadKeyboardBlob } from '../core-adapter/basic.tests.js';
import { Deadkey, SyntheticTextStore } from 'keyman/engine/keyboard';
import { CoreKeyboardProcessor } from 'keyman/engine/core-processor';

import core_context_item = unitTestEndpoints.core_context_item;

const EndContextItem = new core_context_item(KM_CORE_CT.END, 0, 0);

// These tests would run headless if we'd additionally build WASM for node

describe('CoreKeyboardProcessor', function () {
  const loadKeyboard = async function (name: string): Promise<km_core_keyboard> {
    const blob = await loadKeyboardBlob(name)
    const result = KM_Core.instance.keyboard_load_from_blob(name, blob);
    assert.equal(result.status, 0);
    assert.isOk(result.object);
    return result.object;
  };

  const createState = async function (keyboardName: string): Promise<km_core_state> {
    const keyboard = await loadKeyboard(keyboardName);
    const state = KM_Core.instance.state_create(keyboard, []);
    assert.equal(state.status, 0);
    assert.isOk(state.object);
    return state.object;
  };

  const addContextItem = function (contextItems: core_context_item[], c: string | number, isMarker: boolean) {
    let item: core_context_item;
    if (isMarker) {
      item = new core_context_item(KM_CORE_CT.MARKER, 0, c as number);
    } else {
      item = new core_context_item(KM_CORE_CT.CHAR, (c as string).charCodeAt(0), 0);
    }
    contextItems.push(item);
  };

  let coreProcessor: CoreKeyboardProcessor;
  let state: km_core_state;
  let context: km_core_context;
  let textStore: SyntheticTextStore;

  describe('saveMarkersToTextStore', function () {
    before(async function () {
      coreProcessor = new CoreKeyboardProcessor();
      await coreProcessor.init(coreurl);
      state = await createState('/common/test/resources/keyboards/test_8568_deadkeys.kmx');
      context = KM_Core.instance.state_context(state);
      textStore = new SyntheticTextStore('abcd', 3);
    });

    it('saves markers to TextStore', function() {
      // Setup
      // Text index   : 0 1   1   1 2 3   3
      // context index: 0 1   2   3 4 5   6
      // ContextItems : a dk1 dk2 b c dk3 d
      const contextItems: core_context_item[] = [];
      addContextItem(contextItems, 'a', false);
      addContextItem(contextItems, 1, true); // deadkey 1
      addContextItem(contextItems, 2, true); // deadkey 2
      addContextItem(contextItems, 'b', false);
      addContextItem(contextItems, 'c', false);
      addContextItem(contextItems, 3, true); // deadkey 3
      addContextItem(contextItems, 'd', false);
      contextItems.push(EndContextItem);

      sinon.stub(KM_Core.instance, 'context_get').returns({
        status: KM_CORE_STATUS.OK,
        object: {
          items: contextItems,
          delete: function (): void {}
        },
        delete: function (): void {}
      });

      // Execute
      coreProcessor.unitTestEndPoints.saveMarkersToTextStore(context, textStore);

      // Verify
      assert.equal(textStore.deadkeys().count(), 4, 'Should have 3 deadkeys');
      assert.isTrue(textStore.deadkeys().dks[0].match(1, 1));
      assert.isTrue(textStore.deadkeys().dks[1].match(1, 2));
      assert.isTrue(textStore.deadkeys().dks[2].match(3, 3));
    });
  });

  describe('applyContextFromTextStore', function () {
    before(async function () {
      coreProcessor = new CoreKeyboardProcessor();
      await coreProcessor.init(coreurl);
      state = await createState('/common/test/resources/keyboards/test_8568_deadkeys.kmx');
      context = KM_Core.instance.state_context(state);
      textStore = new SyntheticTextStore('abcd', 3);
    });

    it('applies deadkeys from TextStore to Core context', function () {
      // Setup
      // Text index   : 0 1   1   1 2 3   3
      // context index: 0 1   2   3 4 5   6
      // ContextItems : a dk1 dk2 b c dk3 d
      textStore.deadkeys().add(new Deadkey(1, 1)); // before 'b'
      textStore.deadkeys().add(new Deadkey(1, 2));
      textStore.deadkeys().add(new Deadkey(3, 3)); // before 'd'

      // Execute
      coreProcessor.unitTestEndPoints.applyContextFromTextStore(context, textStore);

      // Verify
      const result = KM_Core.instance.context_get(context);
      assert.equal(result.status, KM_CORE_STATUS.OK);
      const items = result.object.items as core_context_item[];
      assert.equal(items.length, 7, 'Should have 7 context items');
      assert.equal(items[0].type, KM_CORE_CT.CHAR);
      assert.equal(items[0].character, 'a'.charCodeAt(0));
      assert.equal(items[1].type, KM_CORE_CT.MARKER);
      assert.equal(items[1].marker, 1);
      assert.equal(items[2].type, KM_CORE_CT.MARKER);
      assert.equal(items[2].marker, 2);
      assert.equal(items[3].type, KM_CORE_CT.CHAR);
      assert.equal(items[3].character, 'b'.charCodeAt(0));
      assert.equal(items[4].type, KM_CORE_CT.CHAR);
      assert.equal(items[4].character, 'c'.charCodeAt(0));
      assert.equal(items[5].type, KM_CORE_CT.MARKER);
      assert.equal(items[5].marker, 3);
      assert.equal(items[6].type, KM_CORE_CT.END);
      result.delete();
    });
  });
});
