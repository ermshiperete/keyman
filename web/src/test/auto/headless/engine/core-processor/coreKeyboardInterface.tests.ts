/*
 * Keyman is copyright (C) SIL Global. MIT License.
 */

import { assert } from 'chai';
import sinon from 'sinon';
import { unitTestEndpoints } from 'keyman/engine/core-processor';
import { VariableStoreTestSerializer } from 'keyman/test/headless-resources';
import { KM_Core, KM_CORE_STATUS, KM_CORE_OPTION_SCOPE } from 'keyman/engine/core-adapter';

describe('CoreKeyboardInterface tests', function () {
  let sandbox: sinon.SinonSandbox;

  beforeEach(function () {
    sandbox = sinon.createSandbox();
  });

  afterEach(function () {
    sandbox.restore();
  });

  describe('loadSerializedOptions', function () {
    it('returns empty array if there are no default options', function () {
      const mockAttrs = {
        object: {
          id: 'test_keyboard',
          version_string: '1.234',
          default_options: [],
          delete: sandbox.stub()
        } as km_core_keyboard_attrs,
        status: KM_CORE_STATUS.OK,
      };
      sandbox.stub(KM_Core.instance, 'keyboard_get_attrs').returns(mockAttrs);
      const keyboardInterface = new unitTestEndpoints.CoreKeyboardInterface(new VariableStoreTestSerializer());
      const options = keyboardInterface['loadSerializedOptions']();
      assert.isArray(options);
      assert.lengthOf(options, 0);
    });

    it('returns options from keyboard attrs', function () {
      const mockAttrs = {
        object: {
          id: 'test_keyboard',
          version_string: '1.234',
          default_options: [
            { key: 'opt1', value: 'val1', scope: KM_CORE_OPTION_SCOPE.OPT_KEYBOARD }
          ],
          delete: sandbox.stub(),
        } as km_core_keyboard_attrs,
        status: KM_CORE_STATUS.OK,
      };
      sandbox.stub(KM_Core.instance, 'keyboard_get_attrs').returns(mockAttrs);
      const keyboardInterface = new unitTestEndpoints.CoreKeyboardInterface(new VariableStoreTestSerializer());
      const options = keyboardInterface['loadSerializedOptions']();
      assert.isArray(options);
      assert.lengthOf(options, 1);
      assert.deepEqual(options[0], { key: 'opt1', value: 'val1', scope: KM_CORE_OPTION_SCOPE.OPT_KEYBOARD });
    });
  });
});