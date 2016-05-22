from rest_framework.viewsets import GenericViewSet, mixins
from speedfeed.models import Raw
from speedfeed.serializers import RawSerializer

class RawViewSet(mixins.CreateModelMixin, GenericViewSet):
    queryset = Raw.objects.all()
    serializer_class = RawSerializer

