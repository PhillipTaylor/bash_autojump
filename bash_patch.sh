#!/bin/bash

#autogen'd script to patch bash source tree
#with the wicked-cool autojump feature.
#
#usage:

#  bash_patch.sh [<path_to_bash_src_tree>]

TARGDIR="."
SRCDIR=$(pwd)

if [ ! -z "$1" ]; then
   TARGDIR=$1
fi
if [ -d "$TARGDIR" ]; then
   if [ "$(basename $TARGDIR)" != "builtins" ]; then
      if [ -d "$TARGDIR/builtins" ]; then
         TARGDIR=$TARGDIR/builtins
      fi
   fi
fi

if [ "$(basename $TARGDIR)" != "builtins" ]; then
   echo "Error: Cannot locate bash src tree."
   exit 1
fi

TARFILE=${SRCDIR}/ajump_tar$$.tgz

##read through to find where the tar file contents begin
line_no=1
while [ 1 ]; do
   read DATA
   [ $? -ne 0 ] && break

   line_no=$((line_no + 1))
   if [ "$DATA" = "__CONTENTS" ]; then
      break
   fi
done < ${0}

echo ${line_no}
tail -n +${line_no} ${0} > ${TARFILE}

cd ${TARGDIR}

tar xvzf ${TARFILE}
rm -f ${TARFILE}
patch -p0 < Makefile.in.patch
patch -p0 < common.c.patch

rm -f Makefile.in.patch
rm -f common.c.patch

exit 0

__CONTENTS
� Q��I �[ys�F�ϿƧx�8)�,)#'Z�k"K.��&.4ɶ@4
�h9�������4)���-�e����w��z��Y����/W�Xvv��뮽n�+���;��흭�f�9�d3�� O��i�% �^�`�������)Oa�Cx��1@&,b��1S�wٗ���X���M\�<�x�B�}�(�+�F�f�9�]�'��~6����V�NE0�n$�g������A�%ݿ�^��%!�ɯC��	�Y�2�E4�9Ϧ�Ss�8C$�6����,��z��|ޝDyW$�^�����]�y������ptaV���G~�8�јO�ӗV���#�����#�������}�r���$+������;�"�]���b�lbQ��r��G<�٬�n��K��@����x�������Q��
O��x������p��]cċR^�Q7�(��D�%I$�U֓����)���ЛԖ>�>�^6�V�b6Q����L�Y����gB�[U�a�*Wr�r؇�%*)Y3�r*�xI�ݹcA�μM�hQ�uu|ry|
?_�^�Ὣ��yz�����=:;���p�ƽw�!�v/II�\$DP���Dr��k&{�H]�%�L�����wh�)vOѲ/��4Tw�4�u� �d|&�ȣ���sx~|9:?>���K��G0�r
���iC�XrP�YT������8.G��K�)��D���f^��p%f"du�gp�V��{�9���H�J+�D})�`�.u>�D��>ɜ���<���PY�j�:t��y��y���i~���1���@�Һ���w��л.��Y�q�j:o����o��e����:��G>��&<�Q�i�x2z�1?��ܥ�"0/lc����uF��<�A8ɣm�:�Z��[�'eY<GsG�#o�Z-�n�n��ဴs�͆����������9�g�^�����: u��k.�r)�f�ͯG�6���G{���h(�@Ӓ�G�x�l�$T�>�"�X��Ф	[jI���<�a������Qz�fdg|~3�*KfF�T�'�㠰��M��ůg�G�	&X�J%Y%lP���eU����d�,�C��RU++���K��$�h�Y̾5\�>�vp�/����u�km�TCU�hK�V�uV;`X�B�[�N�+R,�uv�X0c,��4�}�Vdr%-K��2��&$����A4A��tF]���֤?��PҢ����LӔ+�x�&�n����]�Zo����:�x�xƟ������<����B\�_�c4 ��]*S/��pZ'���Y�6%�_c>��;��M����[l�/�+����e��RE9�^��z]B��f����UXEe
������?�^�q>>��A�73֋���^��!�����T�1M�/��6^ =�S��>"O(��>�# B9^n|����g�x{z�^���óӣhnc���0��
x�?2$�@���ܽ8��<������8s:RۃMǡ$�E��	��#��}O"��d�w%"i�7N!\-KFDd���A-�'�ᤕz]襙����(�|Z�	�t��K[Xѻ!��h����u**�[Ř%�g��Ӡ��ǣ6x��mF��Y6g�Q�$��Ph���Z����=ԕ~�$R��4t�\���0� rIu�w:�U�뾊�/�L�Jn�#��1�'a�٧RLojT%�Da�5o�6�8�l�y��qc=��_6��;�������\b��â�a�Ɔ�/V��P��+�o,��bA���u���Ԟ墴y�C���'�&G����H?�P�l��H�\�{�mW��/]��1+}X�g.���-��D�y�4ٮ���~,()?��0�����`ƒ	��G��I��k�.$��rԢa`�ߙ�q?�'�,c��e��zʼwa�LwBʚ&Gi�jh�MҏAz�lRK��l�l(!�Uf�DL��q�AC�=���Q������̒��x*����<�.��.fHq~v=?9�eh�S��U/F�_p�K�B	:c��*Ā ժMV��x�lU���%ve��v-���pܘ�4���F��J���g��=£D�Mq��b.�(؈U�݂@J^}�c+N��TפUD���j�4Qb'����8�X�3h9Oz2�b8%��z`A���3e-O�~(RV���D�Ȗ�uP��f)�C���z�W)�~بn�^g���:C������@%%�4�5�n�mR�B[����j�*�8����M5y��o;0h[�K�i&qWY�S�{���C�Sc�W'���XW�X���..KPQ��E3"�߇�~�'��.e�؋�?D��Ic/I)¨�	���;�1��d�g��3X��NŸ���+;�Q��DhI>+��A_S�D�bLm�g�	=T/,a�Dɞ��l��D=�����N��S�QwG*~|פQ�@��VM�Ei�R�*E`eВ�QT�M!��8a��^raw����k�����l�4ҧ�2LQxJ��~ن~ۆGIr�q?Ց��Ԯ>Cӧh�.-�e����iu�ydрl���]�Ŝ�
��i "��j^̡u�P�g��:��Qv[��tJ����`�iP��:�SA
9W�B���!����d�ɟ�P��e,�V��bO����v^*CZ�T�Ö�H�u^J��@P�\7{�R�]�R�����Sa������$�@����]!XG[�"��ź�.x�ruk�-��}���vVЫ��� P��¼1��P?<���q���Lz`��+�O�T��c�fטkn���S��Ey�
UXBi��3R�Cj:���=�=�0�A4��<�-�\3�ivL�8���@�<�S��Hb��!� b��
�h�mO�
��H���9��ϲ�"O-Qz����A'�����	�;����b��Ա���	�zA���֝k
<�J�3x|�Z���d	��,���
�]Ό5�"k[�-�5���xw���i=u��szdLլ�ԼT\����E��r�,=#�N�.ᆇ���Kr>+���^P)�7H���Z!3�jcʏ�<s�3�:�B*"	�S�)�L�#0C��}G�C�k���9yXf����kh_f���eO��)T�ٖieX �S�d��;Ԏ��\�o�����O+=�_֖��z�%��+$�P����'�*��5ޭ������yn���<�Ǐ� �"Gk0����>���V(e���)�c~(co+<&_HR���Kr#�d�ԓGae*��)�E��v�˸4���J������<��o3h�-�.�p�D��j(J����p�ʖq�tȫɯ�۪OY�S,��r靍o�����"�`���Ua!��<D�3EO�c5�M����ڎ�~�Xn��[�t�]t1Z��<F^������>|�#��g��Y�?Y��٭t�LR�b��i���d�4`��o9�5tjOݿ���UYV���_n�G���o�����N_=��Z=��+�}[�F�W��_-<9��G����LR�pOPŀ�_����V���~7���e�x������,�;�����������t�ڀ�'��~���鿀�������N��>lЧ���a-��Z��O���j��~���~r��*�y�|Jg���)Wgc�[B����/p=wQ��k��^
$��ww�������%WF�|>A7���,�樰҃]�ٕw�ԫQ��o?n�K�����������Z��g��6�U�j�w���F}��X����1�kkg�����U��K
����S�����>F�ݥ)�4�>����`il�h��]T�}�&>=_��֥�h��ċ�Uw1�U������!���g^zS��{�[��ܝܲ��?��i�.��/��N���׺cC��Z�̋�����ߠ��T��_�QƸ�<�<>t/ή�G��Lv��5^�E��A��ʇү�|)�'�7V�4�=����4�}/�Ӯ�g�xA����bTHJ��y���Ie_���,��)��'����GM�-�,Ki=x%��Y� mx#�@HmI���T�@K�H�6uEy���
��H]Yd#�)��|tp9:��]��h�w
�nP��7������^�b3�e���vTY�UY�UY�UY�UY�UY�UY�UY�UY�UY����?��a� P  