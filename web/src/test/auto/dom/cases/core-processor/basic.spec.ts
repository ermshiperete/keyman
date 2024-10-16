import { assert } from 'chai';
import { CoreKeyboard, CoreProcessor, KM_CORE_STATUS } from 'keyman/engine/core-processor';

const coreurl = '/build/engine/core-processor/obj/import/core';

// Test the CoreProcessor interface.
describe('CoreProcessor', function () {
  async function loadKeyboardBlob(uri: string) {
    const response = await fetch(uri);
    if (!response.ok) {
      throw new Error(`HTTP ${response.status} ${response.statusText}`);
    }

    const buffer = await response.arrayBuffer();
    return new Uint8Array(buffer);
  }

  it('can initialize without errors', async function () {
    const kp = new CoreProcessor();
    assert.isTrue(await kp.init(coreurl));
  });

  it('can call temp function', async function () {
    const kp = new CoreProcessor();
    await kp.init(coreurl);
    const a = kp.tmp_wasm_attributes();
    assert.isNotNull(a);
    assert.isNumber(a.max_context);
    console.dir(a);
  });

  it('can load a keyboard from blob', async function () {
    const kp = new CoreProcessor();
    await kp.init(coreurl);
    const blob = await loadKeyboardBlob('/common/test/resources/keyboards/test_8568_deadkeys.kmx')
    let kb: CoreKeyboard = {};
    const status = kp.km_core_keyboard_load_from_blob('test', blob, kb);
    assert.equal(status, KM_CORE_STATUS.OK);
    assert.isNotNull(kb);
  });
});
