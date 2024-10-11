type km_core_attr = import('./import/core/km-core-interface.js').km_core_attr;

export { km_core_status_codes as KM_CORE_STATUS } from './import/core/core-interface.js';
export type CoreKeyboard = {};

export class CoreProcessor {
  private instance: any;

  /**
   * Initialize Core Processor
   * @param baseurl - The url where km-core.js is located
   */
  public async init(baseurl: string): Promise<boolean> {

    if (!this.instance) {
      try {
        const module = await import(baseurl + '/km-core.js');
        this.instance = await module.default({
          locateFile: function (path: string, scriptDirectory: string) {
            return baseurl + '/' + path;
          }
        });
      } catch (e: any) {
        return false;
      }
    }
    return !!this.instance;
  };

  public tmp_wasm_attributes(): km_core_attr {
    return this.instance.tmp_wasm_attributes();
  }

  public km_core_keyboard_load_from_blob(kb_name: string, blob: Uint8Array, keyboard: CoreKeyboard): KM_CORE_STATUS {
    return null;  // this.instance.km_core_keyboard_load_from_blob(kb_name, blob, blob.length, keyboard);
  }
}
