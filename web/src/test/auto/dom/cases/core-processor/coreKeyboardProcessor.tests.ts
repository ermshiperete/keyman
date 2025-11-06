/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */

import { assert } from 'chai';
import sinon from 'sinon';
import { KM_Core, km_core_context, km_core_keyboard, km_core_state, KM_CORE_CT, KM_CORE_STATUS } from 'keyman/engine/core-adapter';
import { coreurl, loadKeyboardBlob } from '../core-adapter/basic.tests.js';
import { SyntheticTextStore } from 'keyman/engine/keyboard';
import { CoreKeyboardProcessor } from 'keyman/engine/core-processor';

class km_core_context_item {
  type: number;
  character: number;
  marker: number;
}

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

  const addContextItem = function (contextItems: km_core_context_item[], c: string | number, isMarker: boolean) {
    const item = new km_core_context_item();
    if (isMarker) {
      item.marker = c as number;
      item.type = KM_CORE_CT.MARKER;
    } else {
      item.character = (c as string).charCodeAt(0);
      item.type = KM_CORE_CT.CHAR;
    }
    contextItems.push(item);
  };

  describe('saveMarkersToTextStore', function () {
    let coreProcessor: CoreKeyboardProcessor;
    let state: km_core_state;
    let context: km_core_context;
    let textStore: SyntheticTextStore;

    before(async function() {
      coreProcessor = new CoreKeyboardProcessor();
      await coreProcessor.init(coreurl);
      state = await createState('/common/test/resources/keyboards/test_8568_deadkeys.kmx');
      context = KM_Core.instance.state_context(state);
      textStore = new SyntheticTextStore('abcd', 3);
    });

    it('saves markers to TextStore', function() {
      // Setup
      const contextItems: km_core_context_item[] = [];
      addContextItem(contextItems, 'a', false);
      addContextItem(contextItems, 1, true); // deadkey 1
      addContextItem(contextItems, 2, true); // deadkey 2
      addContextItem(contextItems, 'b', false);
      addContextItem(contextItems, 'c', false);
      addContextItem(contextItems, 3, true); // deadkey 3
      addContextItem(contextItems, 'd', false);

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
      assert.equal(textStore.deadkeys().count(), 3, 'Should have 3 deadkeys');
      assert.isTrue(textStore.deadkeys().dks[0].match(1, 1));
      assert.isTrue(textStore.deadkeys().dks[1].match(2, 2));
      assert.isTrue(textStore.deadkeys().dks[2].match(5, 3));
    });
  });
});
